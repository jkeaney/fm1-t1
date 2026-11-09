var reference_list = [0, 2, 4, 5, 7, 9, 11];

inlets = 2;
outlets = 1;

function list() {
    if (inlet == 0) {
        // Left inlet - process and output
        var input_list = arrayfromargs(arguments);
        var result = [];
        
        for (var i = 0; i < input_list.length; i++) {
            var closest = reference_list[0];
            var min_distance = Math.abs(input_list[i] - closest);
            
            for (var j = 1; j < reference_list.length; j++) {
                var distance = Math.abs(input_list[i] - reference_list[j]);
                if (distance < min_distance) {
                    min_distance = distance;
                    closest = reference_list[j];
                }
            }
            result.push(closest);
        }
        
        outlet(0, result);
    } else if (inlet == 1) {
        reference_list = arrayfromargs(arguments);
    }
}
