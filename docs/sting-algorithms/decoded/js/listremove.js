inlets = 1;
outlets = 1;

// stored list
var stored_list = [];

// handle "list" messages
function list() {
    // arguments contains all the numbers after "list"
    stored_list = arrayfromargs(arguments);
    output_list();
}

// handle "remove" messages
function remove(val1, val2) {
    if (arguments.length !== 2) {
        error("remove requires exactly 2 arguments\n");
        return;
    }
    
    var new_list = [];
    var i = 0;
    
    // iterate through the list in pairs
    while (i < stored_list.length) {
        // check if we have a complete pair
        if (i + 1 < stored_list.length) {
            // if this pair doesn't match the pair to remove, keep it
            if (!(stored_list[i] === val1 && stored_list[i + 1] === val2)) {
                new_list.push(stored_list[i]);
                new_list.push(stored_list[i + 1]);
            }
            i += 2; // move to next pair
        } else {
            // odd number at the end, keep it
            new_list.push(stored_list[i]);
            i += 1;
        }
    }
    
    stored_list = new_list;
    output_list();
}

// output the current list
function output_list() {
    if (stored_list.length > 0) {
        outlet(0, "list", stored_list);
    }
}

// handle bang to output current list
function bang() {
    output_list();
}

// clear the stored list
function clear() {
    stored_list = [];
}
