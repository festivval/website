const inactive_items = new Set(); 
const active_tags = new Set();

$(document).ready(function() {
	tags.forEach(addTags);
	active_tags.add("tagless");

	$('button#all-filters').on("click", function() {
		inactive_items.clear();
		$('input.filter[type=checkbox]').prop('checked', true);
		display();
	});
	$('button#nil-filters').on("click", function() {
		items.forEach(addItemGroups);
		$('input.filter[type=checkbox]').prop('checked', false);
		display();
	});

	$('button#all-tags').on("click", function() {
		tags.forEach(addTags);
		$('input.tfilter[type=checkbox]').prop('checked', true);
		display();
	});
	$('button#nil-tags').on("click", function() {
		active_tags.clear();
		active_tags.add("tagless");
		$('input.tfilter[type=checkbox]').prop('checked', false);
		display();
	});

	$('input.filter[type=checkbox]').change(function() {
		if (this.checked) {
			var inner_items = items.get($(this).attr('name'));
			inner_items.forEach(deleteItems);
		} else {
			var inner_items = items.get($(this).attr('name'));
			inner_items.forEach(addItems);
		}
		display();
		//inactive_items.forEach(logItems);
	});


	$('input.tfilter[type=checkbox]').change(function() {
		if (this.checked) {
			active_tags.add($(this).attr('name'));
		} else {
			active_tags.delete($(this).attr('name'));
		}
		display();
		//active_tags.forEach(logItems);
	});
});

const display = function() {
	$('li').hide();
	active_tags.forEach(displayTags);
	inactive_items.forEach(displayItems);
}

const addItemGroups = function(val) {
	val.forEach(addItems);
}

const addItems = function(val) {
	inactive_items.add(val);
}

const addTags = function(val) {
	active_tags.add(val);
}

const deleteItems = function(val) {
	inactive_items.delete(val);
}

const displayTags = function(val) {
	if(active_tags.has(val)) {
		$('.'+val).parent().show();
	}
}

const displayItems = function(val) {
	if(inactive_items.has(val)) {
		$('.'+val).parent().hide();
	}
}

/*const vdisplayItems = function(val) {
	if(inactive_items.has(val)) {
		$('.'+val).parent().hide();
		console.log("hiding "+val);
	}
}*/

const logItems = function(val) {
	console.log(val);
}
