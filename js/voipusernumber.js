Drupal.behaviors.click2call = function (context) {
	$(".hangup-button").attr('disabled', 'disabled');
	$(".click2call-hidden").hide();
	click2call_attach_events();
	$(".click2call-hangup-button").attr('disabled', 'disabled');
}

function click2call_attach_events(){  
	//Automatically select radio when typing in textfield
	$(".click2call-type").focus(function() { 
		$(this).prev().attr("checked","checked");
	});  
	
	//Automatically select radio when focusing on select list
	$(".click2call-voipnumber").focus(function() { 
		$(this).prev().attr("checked","checked");
	});  
}

function click2call_display(field_name,delta) { 
  if($("#click2call-group-"+field_name+"-"+delta).hasClass('on')){
    $("#click2call-group-"+field_name+"-"+delta).hide();	
  }
  else{
    $("#click2call-group-"+field_name+"-"+delta).show();
  }
  
  $("#click2call-group-"+field_name+"-"+delta).toggleClass('on');
}


function click2call_call(field_name,delta,nid) {  
	//Disable the button
	$("#click2call-button-"+field_name+"-"+delta).attr('disabled', 'disabled');	
    //Remove error message
	$("#click2call-"+field_name+"-"+delta+"-status").html("");
	//Upload button
	var phone=$("#click2call-group-"+field_name+"-"+delta+" input:radio:checked").val();
	if(phone=="voipnumber"){
		//Get the phone number from select list
		phone=$("#click2call-"+field_name+"-"+delta+"-select :selected").val();
		console.log('in voip number');
		console.log(phone);
	}
	else if(phone=="type"){
		phone=$("#click2call-"+field_name+"-"+delta+"-type-phone").val();
		console.log('in type');
		console.log(phone);
	}
	
	//Hide the phone number selection
	$("#click2call-"+field_name+"-"+delta+"-hidden").html($(".click2call-"+field_name+"-"+delta+"-phone").html());
    $(".click2call-"+field_name+"-"+delta+"-phone").html("Calling...");
		//Enable hangup button
		$("#click2call-hangup-button-"+field_name+"-"+delta).removeAttr("disabled");	
		$.ajax({
			type: "GET",
			url: Drupal.settings.basePath +"?q=voipnumber/call",
			data: "phone=" + encodeURIComponent(phone)+"&field_name="+field_name+"&delta="+delta+"&nid="+nid,
			dataType: 'json',
			success: function(data){
			console.log('i am in sucess');
						click2call_response(data,field_name,delta);
					}
		});
} 

function click2call_hangup(field_name,delta){
	var call_nid=$("#click2call-"+field_name+"-"+delta+"-callnid").val();
	
	$.ajax({
			type: "GET",
			url: Drupal.settings.basePath +"?q=voipnumber/hangup",
			data: "call_nid=" + call_nid,
			dataType: 'json',  
			success: function(data){
						if(data.status){
							$(".click2call-"+field_name+"-"+delta+"-phone").html("Terminating call...");
						}	
					}
	});
}

function click2call_response(data,field_name,delta){
	$("#click2call-"+field_name+"-"+delta+"-callnid").val(data.call_nid);
	// Check for call status after 3 sec.
	window.setTimeout(function() {
		click2call_check(data.call_nid,field_name,delta);
	}, 3000);

}

function click2call_check(call_nid,field_name,delta){
$.ajax({
		type: "GET",
		url: Drupal.settings.basePath +"?q=voipnumber/get/status",
		data: "call_nid=" + call_nid,
		dataType: 'json',
		success: function(data){
					if(data.status=="success"){
						//Call is sucess
						$(".click2call-"+field_name+"-"+delta+"-phone").html($("#click2call-"+field_name+"-"+delta+"-hidden").html());
						$("#click2call-"+field_name+"-"+delta+"-hidden").html("");
						click2call_attach_events();
						$("#click2call-"+field_name+"-"+delta+"-status").html(data.message);
						$("#click2call-button-"+field_name+"-"+delta).removeAttr("disabled");
						$("#click2call-hangup-button-"+field_name+"-"+delta).attr('disabled', 'disabled');
					}
					else if(data.status=="failed"){
						//Call is terminated (busy, not answered, error, unavailable, ...)
						$(".click2call-"+field_name+"-"+delta+"-phone").html($("#click2call-"+field_name+"-"+delta+"-hidden").html());
						$("#click2call-"+field_name+"-"+delta+"-hidden").html("");
						click2call_attach_events();
						$("#click2call-"+field_name+"-"+delta+"-status").html(data.message);
						$("#click2call-button-"+field_name+"-"+delta).removeAttr("disabled");	
						$("#click2call-hangup-button-"+field_name+"-"+delta).attr('disabled', 'disabled');					
					}
					else{
					//Call is not finished yet, check again after 2 sec.
						window.setTimeout(function() {
							click2call_check(call_nid,field_name,delta);
						}, 2000);
					}
				}
	});
}
