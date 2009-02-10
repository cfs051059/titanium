//
// load projects when manage projects is clicked
//
var TFS = Titanium.Filesystem;
var TiDeveloper  = {};
TiDeveloper.currentPage = 1;
TiDeveloper.init = false;

// holder var for all projects
TiDeveloper.ProjectArray = [];
var db = openDatabase("TiDeveloper","1.0");
var highestId = 0;

function formatCountMessage(count,things)
{
	return (count == 0) ? 'You have no '+things+'s' : count == 1 ? 'You have 1 '+things : 'You have ' + count + ' '+things+'s';
}

// State Machine for UI tab state
TiDeveloper.stateMachine = new App.StateMachine('ui_state');
TiDeveloper.stateMachine.addState('manage','l:menu[val=manage]',false);
TiDeveloper.stateMachine.addState('create','l:menu[val=create]',false);
TiDeveloper.stateMachine.addState('api','l:menu[val=api]',false);
TiDeveloper.stateMachine.addState('interact','l:menu[val=interact]',true);
TiDeveloper.currentState = null;
TiDeveloper.stateMachine.addListener(function()
{
	TiDeveloper.currentState = this.getActiveState();
	if (TiDeveloper.currentState != 'interact')
	{
		TiDeveloper.startIRCTrack();
	}
	else
	{
		TiDeveloper.stopIRCTrack();
		
	}
});
TiDeveloper.ircMessageCount = 0;
TiDeveloper.startIRCTrack = function()
{
	TiDeveloper.ircMessageCount = 0;
	$('#irc_message_count').html('');
	$('#irc_message_count').css('display','inline');
	
};
TiDeveloper.stopIRCTrack = function()
{
	TiDeveloper.ircMessageCount = 0;
	$('#irc_message_count').css('display','none');
	$('#irc_message_count').html('');
	
};

function createRecord(name,dir,appid)
{
	var record = {
		name: name,
		dir: dir,
		id: highestId++,
		appid: appid,
		date: new Date().getTime()
	};
	TiDeveloper.ProjectArray.push(record);
    db.transaction(function (tx) 
    {
        tx.executeSql("INSERT INTO Projects (id, timestamp, name, directory, appid) VALUES (?, ?, ?, ?, ?)", [record.id, record.date, record.name, record.dir, record.appid]);
    });
}

function loadProjects()
{
	db.transaction(function(tx) 
	{
        tx.executeSql("SELECT id, timestamp, name, directory, appid FROM Projects", [], function(tx, result) 
		{
			TiDeveloper.ProjectArray = [];
            for (var i = 0; i < result.rows.length; ++i) {
                var row = result.rows.item(i);
				// check to see if the user has deleted it and if
				// so remove it
				var cd = TFS.getFile(row['directory']);
				if (!cd.exists())
				{
					tx.executeSql('DELETE FROM Projects where id = ?',[row['id']]);
				}
				else
				{
					TiDeveloper.ProjectArray.push({
						id: row['id'],
						date: row['timestamp'],
						name: row['name'],
						dir: row['directory'],
						appid: row['appid']
					});
					if (highestId < row['id'])
					{
						highestId = row['id'];
					}
				}
            }
			TiDeveloper.currentPage = 1;
			var data = TiDeveloper.getProjectPage(10,TiDeveloper.currentPage);
			var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
			$MQ('l:project.list.response',{count:count,page:TiDeveloper.currentPage,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
        }, function(tx, error) {
            alert('Failed to retrieve projects from database - ' + error.message);
            return;
        });
	});	
}

db.transaction(function(tx) 
{
   tx.executeSql("SELECT COUNT(*) FROM Projects", [], function(result) 
   {
       loadProjects();
   }, function(tx, error) 
   {
       tx.executeSql("CREATE TABLE Projects (id REAL UNIQUE, timestamp REAL, name TEXT, directory TEXT, appid TEXT)", [], function(result) 
	   { 
          loadProjects(); 
       });
   });
});

//
//  create.project service
//
$MQL('l:create.project.request',function(msg)
{
	try
	{
		var result = Titanium.Project.create(msg.payload.project_name,msg.payload.project_location);
		if (result.success)
		{
			createRecord(result.name,result.basedir,result.id);
			$MQ('l:create.project.response',{result:'success'});
			var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
			$MQ('l:project.list.response',{count:count,page:1,totalRecords:TiDeveloper.ProjectArray.length,'rows':TiDeveloper.ProjectArray})
		}
		else
		{
			$MQ('l:create.project.response',{result:'error',message:result.message});
		}
	}
	catch(E)
	{
		alert('Exception = '+E);
	}
});

//
// Handling paging requests
//
$MQL('l:page.data.request',function(msg)
{
	var state =msg.payload
	var rowsPerPage = state.rowsPerPage
	var page = state.page
	TiDeveloper.currentPage = page;
	var data = TiDeveloper.getProjectPage(rowsPerPage,page);
	var count = formatCountMessage(TiDeveloper.ProjectArray.length,'project');
	$MQ('l:project.list.response',{count:count,page:page,totalRecords:TiDeveloper.ProjectArray.length,'rows':data})
});

TiDeveloper.formatDirectory =function(dir)
{
	var dirStr = dir
	if (dir.length > 40)
	{
		dirStr = dir.substring(0,40) + '...';
		$('#project_detail_dir_a').css('display','block');
		$('#project_detail_dir_span').css('display','none');
		$('#project_detail_dir_a').html(dirStr);
	}
	else
	{
		$('#project_detail_dir_span').css('display','block');
		$('#project_detail_dir_a').css('display','none');
		$('#project_detail_dir_span').html(dirStr);
	}
}
//
// Get a page of data
//
TiDeveloper.getProjectPage = function(pageSize,page)
{
	var pageData = [];
	var start = (page==0)?0:(pageSize * page) - pageSize;
	var end = ((pageSize * page) > TiDeveloper.ProjectArray.length)?TiDeveloper.ProjectArray.length:(pageSize * page);
	for (var i=start;i<end;i++)
	{
		pageData.push(TiDeveloper.ProjectArray[i])
	}
	return pageData
};

var modules = [];
var module_map = {};

setTimeout(function()
{
	var result = Titanium.Project.getModulesAndRuntime();
	modules.push({name:'Titanium Runtime',versions:result.runtime.versions,dir:result.runtime.dir});
	for (var c=0;c<result.modules.length;c++)
	{
		var name = result.modules[c].name;
		module_map[name]=result.modules[c];
		modules.push({name:name,versions:result.modules[c].versions,dir:result.modules[c].dir});
	}
},500);

//
//  Project Package Request - get details about modules, etc
//
$MQL('l:package.project.request',function(msg)
{
	try
	{
		$MQ('l:package.project.data',{rows:modules});
		$MQ('l:package.all',{val:'network'});
	}
	catch (E)
	{
		alert("Exception = "+E);
	}
});

function findProject(name)
{
	for (var i=0;i<TiDeveloper.ProjectArray.length;i++)
	{
		if (TiDeveloper.ProjectArray[i].name == name)
		{
			return TiDeveloper.ProjectArray[i];
		}
	}
	return null;
}

//
// Create Package Request
//
$MQL('l:create.package.request',function(msg)
{
	try
	{
		// elements that are included for network bundle
		var networkEl = $("div[state='network']");

		// elements that are included (bundled)
		var bundledEl = $("div[state='bundled']");

		// elements that are excluded
		var excludedEl = $("div[state='exclude']");
		
		var excluded = {};
		
		$.each(excludedEl,function()
		{
			var key = $.trim($(this).html());
			excluded[key]=true;
		});
		
		// project name
		var project_name = $('#package_project_name').html();

		var launch = msg.payload.launch;

		var project = findProject(project_name);

		// build the manifest
		var manifest = 'appname:'+project_name+'\n';
		manifest+='appid:'+project.appid+'\n';
		manifest+='runtime:'+modules[0].versions[0]+'\n';
		
		// 0 is always runtime, skip it
		for (var c=1;c<modules.length;c++)
		{
			if (!excluded[modules[c].name])
			{
				manifest+=modules[c].name+':'+modules[c].versions[0]+'\n';
			}
		}
		
		var mf = TFS.getFile(project.dir,'manifest');
		mf.write(manifest);
		
		var dist = TFS.getFile(project.dir,'dist',Titanium.platform);
		dist.createDirectory(true);
		
		var runtime = TFS.getFile(modules[0].dir,modules[0].versions[0]);
		
		//TODO: toggle installed flag here based on testing installer or not
		var installed = true;
		var app = Titanium.createApp(runtime,dist,project_name,project.appid,installed);
		var app_manifest = TFS.getFile(app.base,'manifest');
		app_manifest.write(manifest);
		var resources = TFS.getFile(project.dir,'resources');
		var tiapp = TFS.getFile(project.dir,'tiapp.xml');
		tiapp.copy(app.base);
		var launch_fn = function()
		{
			if (launch)
			{
				Titanium.Desktop.openApplication(app.executable.nativePath());
			}
		};
		TFS.asyncCopy(resources,app.resources,function()
		{
			var module_dir = TFS.getFile(app.base,'modules');
			var runtime_dir = TFS.getFile(app.base,'runtime');
			var modules_to_bundle = [];
			$.each(bundledEl,function()
			{
				var key = $.trim($(this).html());
				var target, dest;
				if (key == 'Titanium Runtime') //TODO: we need to make this defined
				{
					runtime_dir.createDirectory();
					target = TFS.getFile(modules[0].dir,modules[0].versions[0]);
					dest = runtime_dir;
				}
				else
				{
					module_dir.createDirectory();
					var module = module_map[key];
					target = TFS.getFile(module.dir,module.versions[0]);
					dest = TFS.getFile(module_dir,module.dir.name());
				}
				modules_to_bundle.push({target:target,dest:dest});
			});
			
			if (modules_to_bundle.length > 0)
			{
				var count = 0;
				for (var c=0;c<modules_to_bundle.length;c++)
				{
					var e = modules_to_bundle[c];
					TFS.asyncCopy(e.target,e.dest,function(filename,c,total)
					{
						if (++count==modules_to_bundle.length)
						{
							// link libraries if runtime included
							if (e.dest == runtime_dir)
							{
								Titanium.linkLibraries(e.dest);
							}
							launch_fn();
						}
					});
				}
			}
			else
			{
				// no modules to bundle, installer the net installer
				var net_installer_src = TFS.getFile(runtime,'installer');
				var net_installer_dest = TFS.getFile(app.base,'installer');
				TFS.asyncCopy(net_installer_src,net_installer_dest,function(filename,c,total)
				{
					launch_fn();
				});
			}
			
		});
	}
	catch(E)
	{
		alert("Exception = "+E);
	}
})

//
//  Delete a project
//	
$MQL('l:delete.project.request',function(msg)
{
	var name = msg.payload.name;
	var id = msg.payload.id
	if (confirm('Are you sure you want to delete project: ' + name + '?')==true)
	{
		db.transaction(function (tx) 
	    {
	        tx.executeSql("DELETE FROM Projects where id = ?", [id]);
			loadProjects();
	    });
	}
});

//
// project search request
//
$MQL('l:project.search.request',function(msg)
{
	var q = msg.payload.search_value;

	db.transaction(function(tx) 
	{
		try
		{
	        tx.executeSql("SELECT id, timestamp, name, directory FROM Projects where name LIKE '%' || ? || '%'", [q], function(tx, result) 
			{
				try
				{
					var results = [];
		            for (var i = 0; i < result.rows.length; ++i) {
		                var row = result.rows.item(i);
						results.push({
							id: row['id'],
							date: row['timestamp'],
							name: row['name'],
							dir: row['directory']
						});
					}
					$MQ('l:project.search.response',{totalRecords:results.length,'rows':results});
				}
				catch (EX)
				{
					alert("EXCEPTION = "+EX);
				}
			});
		}
		catch (E)
		{
			alert("E="+e);
		}
	});
});

//
// Show file dialog and send value
//
$MQL('l:show.filedialog',function()
{
	var files = Titanium.UI.openFiles({directories:true});
	var val = files.length ? files[0] : '';
	$MQ('l:file.selected',{value:val});
});

var irc_count = 0;

setTimeout(function()
{
	try
	{
		var myNick = Titanium.Platform.username+'1';
		var myName = Titanium.Platform.username+'1';
		var myNameStr = Titanium.Platform.username+'1';

		$('#irc').append('<div style="color:#aaa">you are joining the room. one moment...</div>');
		
		var irc = Titanium.Network.createIRCClient();
		irc.connect("irc.freenode.net",6667,myNick,myName,myNameStr,String(new Date().getTime()),function(cmd,channel,data,nick)
		{
			// switch on command
			switch(cmd)
			{
				case 'NOTICE':
				case 'PRIVMSG':
				{
					if (nick && nick!='NickServ')
					{
						if (TiDeveloper.currentState != 'interact') TiDeveloper.ircMessageCount ++;
						$('#irc_message_count').html(TiDeveloper.ircMessageCount);
						$('#irc').append('<div style="color:yellow">' + nick + ': ' + channel.substring(1,channel.length) + '</div>');
					}
					break;
				}
				case '366':
				{					
					var users = irc.getUsers('#titanium_dev');
					$MQ('l:online.count',{count:users.length});
					irc_count = users.length;
					for (var i=0;i<users.length;i++)
					{
						if (users[i].operator == true)
						{
							$('#irc_users').append('<div class="'+users[i].name+'" style="color:#457db3">'+users[i].name+'(op)</div>');
						}
						else if (users[i].voice==true)
						{
							$('#irc_users').append('<div class="'+users[i].name+'" style="color:#457db3">'+users[i].name+'(v)</div>');
						}
						else
						{
							$('#irc_users').append('<div class="'+users[i].name+'">'+users[i].name+'</div>');
						}
					}
				}
				case 'JOIN':
				{
					if (nick.indexOf('freenode.net') != -1)
					{
						continue;
					}
					
					if (nick == myNick)
					{
						$('#irc').append('<div style="color:#aaa"> you are now in the room. </div>');
						break
					}
					else
					{
						irc_count++;
					}
					$('#irc').append('<div style="color:#aaa">' + nick + ' has joined the room </div>');
					$('#irc_users').append('<div class="'+nick+'" style="color:#457db3">'+nick+'</div>');
					$MQ('l:online.count',{count:irc_count});
					break;
					
				}
				case 'QUIT':
				case 'PART':
				{
					$('#irc').append('<div style="color:#aaa">' + nick + ' has left the room </div>');
					$('.'+nick).html('');
					irc_count--;
					$MQ('l:online.count',{count:irc_count});
					break;
				}
			}
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;
		});

		irc.join("#titanium_dev");
		$MQL('l:send.irc.msg',function()
		{
			irc.send('#titanium_dev',$('#irc_msg').val());
			$('#irc').append('<div style="color:#fff">'+myNick + ': ' + $('#irc_msg').val() + '</div>');
			$('#irc_msg').val('');
			$('#irc').get(0).scrollTop = $('#irc').get(0).scrollHeight;

		})
	}
	catch(E)
	{
		alert("Exception: "+E);
	}
},1000);

