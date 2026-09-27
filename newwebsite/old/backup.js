const inactive_items = new Set();
const active_tags = new Set();

const items = new Map([["personal-group", ["personal"]], ["hackathon-group", ["CipherHacks-2026", "SB-Hacks-XII", "SB-Hacks-XI", "Hack-the-Wave"]], ["club-group", ["Science-Olympiad-Inc", "Quizbowl", "ACM-at-UCSB", "Science-Olympiad-Club"]], ["other-group", ["Gateways-Summer-School"]], ["school-group", ["UCSB-CS", "COMP-4411", "CS-274", "CSCS-130E", "CS-185", "CS-170", "CS-162", "CS-160", "CS-171", "ECE-184", "AP-CSA", "AP-CSP"]]]);
const tags = new Set(["Scratch", "libGDX", "Google_Sheets", "Selenium", "Electron", "C++", "OpenGL", "CSS", "HTML", "Rust", "Java", "OCaml", "JavaScript", "Figma", "Google_Slides", "Python", "C", "Unity"]);

$(document).ready(function() {
	tags.forEach(fillItems);	

	$('input.filter[type=checkbox]').change(function() {
		if (this.checked) {
			var inner_items = items.get($(this).attr('name'));
			inner_items.forEach(deleteItems);
		} else {
			var inner_items = items.get($(this).attr('name'));
			inner_items.forEach(addItems);
		}
		$('li').hide();
		tags.forEach(tdisplayItems);
		items.forEach(displayItems);
		//inactive_items.forEach(logItems);
	});


	$('input.tfilter[type=checkbox]').change(function() {
		if (this.checked) {
			active_tags.add($(this).attr('name'));
		} else {
			active_tags.delete($(this).attr('name'));
		}
		$('li').hide();
		tags.forEach(tdisplayItems);
		items.forEach(displayItems);
		active_tags.forEach(logItems);
	});
});

const fillItems = function(val) {
	active_tags.add(val);
}

const addItems = function(val) {
	inactive_items.add(val);
}

const deleteItems = function(val) {
	inactive_items.delete(val);
}

const tdisplayItems = function(val) {
	if(active_tags.has(val)) {
		$('.'+val).parent().show();
	}
}

const vdisplayItems = function(val) {
	if(inactive_items.has(val)) {
		$('.'+val).parent().hide();
		console.log("hiding "+val);
	}
}
const displayItems = function(val, key, set) {
	val.forEach(vdisplayItems);
}

const logItems = function(val) {
	console.log(val);
}
