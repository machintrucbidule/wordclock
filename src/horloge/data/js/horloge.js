$(document).ready(function() {
	
	//Initiate calculated dropdown menus
	initiateDropDowns();
	
	//Load json data
	$.getJSON("config.json", function(data){
		fetchConfig(data);
		
		//Initiate components
		initiateComponents();
		
		//Hide page loader
		showLoader(false);
		
		setInterval(updateLightLevel, 2000);
		setInterval(updateRebootInfo, 2000);
	});
});

var updateOngoing = false;

function fetchConfig(data){
	$("#timezone").val(data["timezone"]);
	$("input[name=dst][value=" + data["dst"] + "]").prop('checked', true);
	
	$("input[name=colors_hourmin_mode][value=" + data["colors"]["hourmin_mode"] + "]").prop('checked', true);
	$("input[name=colors_seconds_mode][value=" + data["colors"]["seconds_mode"] + "]").prop('checked', true);
	$("input[name=colors_seconds_display][value=" + data["colors"]["seconds_display"] + "]").prop('checked', true);
	$("#colors_back").val("rgb(" + data["colors"]["back"]["r"] + "," + data["colors"]["back"]["g"] + "," + data["colors"]["back"]["b"] + ")");
	$("#colors_hours_on").val("rgb(" + data["colors"]["hours_on"]["r"] + "," + data["colors"]["hours_on"]["g"] + "," + data["colors"]["hours_on"]["b"] + ")");
	$("#colors_minutes_on").val("rgb(" + data["colors"]["minutes_on"]["r"] + "," + data["colors"]["minutes_on"]["g"] + "," + data["colors"]["minutes_on"]["b"] + ")");
	$("#colors_seconds_on").val("rgb(" + data["colors"]["seconds_on"]["r"] + "," + data["colors"]["seconds_on"]["g"] + "," + data["colors"]["seconds_on"]["b"] + ")");
	$("#colors_saturation").val(data["colors"]["saturation"]);
	$("#colors_offset").val(data["colors"]["offset"]);
	
	$("#brightness_day_back").val(data["brightness"]["day"]["back"]);
	$("#brightness_day_hours").val(data["brightness"]["day"]["hours"]);
	$("#brightness_day_seconds").val(data["brightness"]["day"]["seconds"]);
	$("#brightness_night_back").val(data["brightness"]["night"]["back"]);
	$("#brightness_night_hours").val(data["brightness"]["night"]["hours"]);
	$("#brightness_night_seconds").val(data["brightness"]["night"]["seconds"]);
	$("#brightness_threshold").val(data["brightness"]["threshold"]);
	
	$("#hostname").val(data["hostname"]);
	$("#ntp_server").val(data["ntp_server"]);
	$("#restart_day").val(data["restart"]["day"]);
	$("#restart_hour").val(data["restart"]["hour"]);
}

function initiateDropDowns(){
	//Timezones
	for(i =-12; i <= 12; i++){
		var text = i;
		if(i >= 0){
			text = "+" + i;
		}
		$("#timezone").append(
			$('<option></option>').val(i).html("UTC " + text)
		);
	}
	
	//restart_hours
	for(i=0; i<=23;i++){
		$("#restart_hour").append(
			$('<option></option>').val(i).html(i+"h")
		);
	}
}

function initiateComponents(){
	$('input.color').colorPicker({
		forceAlpha:false,
		preventFocus:true,
		renderCallback: function($elm, toggled) {
			if (toggled === false){
				console.log('Color updated');
				ajaxUpdate();
			}
		}
	});
	
	$('input[type=radio]').checkboxradio({
      icon: false
    });
	$('input[type=submit]').button();
	$('#restartbtn').button();
	$('#restartbtn').click(function(){
		$("#restart").val("1");
		$("#horlogeform").submit();
	});
	$('#restoreconfigbtn').button();
	$('#restoreconfigbtn').click(function(){
		if (confirm('Êtes-vous sûr de vouloir restaurer la configuration et redémarrer ?')) {
			$("#restoreconfig").val("1");
			$("#horlogeform").submit();
		}
	});
	$('select').selectmenu({
		width: 150
	});
	$('input.textbox').button().css({'font' : 'inherit','color' : '#35363a','text-align' : 'left','outline' : 'none','cursor' : 'text'});

	$('.divSlider_360').each(function() {
		var $this = $(this);
		$(this).find('div').slider({
			value: $('input',$this).val(),
			min:0,
			max:360,
			range: 'min',
			animate: true,
			slide: function(event, ui) {
				$('input',$this).val(ui.value);
			}, stop: function(event, ui) {
				console.log('slider updated');
				ajaxUpdate();
			}
		});
	});
	$('.divSlider_255').each(function() {
		var $this = $(this);
		$(this).find('div').slider({
			value: $('input',$this).val(),
			min:0,
			max:255,
			range: 'min',
			animate: true,
			slide: function(event, ui) {
				$('input',$this).val(ui.value);
			}, stop: function(event, ui) {
				console.log('slider updated');
				ajaxUpdate();
			}
		});
	});
	
	$('input,select').change(function(){
		if($(this).attr('name') != 'hostname'){
			ajaxUpdate();
		}
	});
	
	$('select').on('selectmenuchange', function(){
		ajaxUpdate();
	});
	
	$("#menu").accordion({
	  header: "h1",
	  heightStyle: "fill"/*,
	  activate: function(event, ui) {
		if(ui.newHeader[0]){
			if( ui.newHeader[0].id =='menu_advanced'){
				console.log("open advanced");
				updateRebootInfo()
			}
		}
	}*/
	});
	$(window).resize(function() {
	// update accordion height
	$("#menu").accordion("refresh");
	});
}

function showLoader(display){
	if(display){
		$('.loader').css('display', 'block');
	}else{
		$('.loader').css('display', 'none'); 
	}
}

function ajaxUpdate(){
	if(!updateOngoing){
		updateOngoing = true;
		showLoader(true);
		$("#dynamic").val("1");
		console.log('start update');
		$.post($('#horlogeform').attr('action'),
			$('#horlogeform').serialize()
		).always(function(){
			updateOngoing=false;
			showLoader(false);
			console.log('end of update');
		});
		$("#dynamic").val("0");
	}
}

function updateLightLevel(){
	if($("#updateLightLevel").is(":visible")){
		console.log("light visible");
			$.getJSON( "getLightLevel.json", function( data ) {
				$("#averagelightlevel").html(data["average"]);
				$("#lastlightlevel").html(data["last"]);
			});
	}else{
		console.log("light hidden");
	}
}

function updateRebootInfo(){
	if($("#lastrebootime").is(":visible")){
		console.log("reboot info visible");
			$.getJSON("getResetInfo.json", function(dataReboot){
				$("#lastrebootime").html(dataReboot["time"]);
				$("#lastrebootreason").html(dataReboot["reason"]);
			});
	}else{
		console.log("reboot info hidden");
	}
}