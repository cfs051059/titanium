$(document).ready(function()	
{
	$('#text_editor').markItUp(mySettings);
});


$MQL('l:launch.sandbox',function(msg)
{
	var project = {};
	project.name = "sandbox";
	project.url = "http://www.titaniumapp.com";
	project.rootdir = Titanium.Filesystem.createTempDirectory().toString();
	project.dir = project.rootdir +'/'+ project.name;
	project.appid = 'com.titanium.sandbox';
	project.publisher = "Titanium";

	var jsLibs = {jquery:false,jquery_ui:false,prototype_js:false,scriptaculous:false,dojo:false,mootools:false,swf:false,yui:false};
	if ($('#jquery_js').hasClass('selected_js'))
	{
		jsLibs.jquery = true;
	}
	if ($('#jqueryui_js').hasClass('selected_js'))
	{
		jsLibs.jquery_ui = true;
	}
	if ($('#prototype_js').hasClass('selected_js'))
	{
		jsLibs.prototype_js = true;
	}
	if ($('#scriptaculous_js').hasClass('selected_js'))
	{
		jsLibs.scriptaculous = true;
	}
	if ($('#dojo_js').hasClass('selected_js'))
	{
		jsLibs.dojo = true;
	}
	if ($('#mootools_js').hasClass('selected_js'))
	{
		jsLibs.mootools = true;
	}
	if ($('#swfobject_js').hasClass('selected_js'))
	{
		jsLibs.swf = true;
	}
	if ($('#yahoo_js').hasClass('selected_js'))
	{
		jsLibs.yahoo = true;
	}

	var outdir = TFS.getFile(project.rootdir,project.name);
	if (outdir.isDirectory())
	{
		outdir.deleteDirectory(true);
	}
	Titanium.Project.create(project.name,project.rootdir,project.publisher,project.url,null,jsLibs, $('#text_editor').val());
	TiDeveloper.Projects.launchProject(project,false)
})
