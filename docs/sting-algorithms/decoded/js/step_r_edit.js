inlets = 1;
outlets = 1;
var lastStart = -1;
var lastEnd = -1;
var currentStep = 0;  

// Set to run in low priority background thread
max.threadpriority = -1;

var prevValues = new Array(16);
for (var i = 0; i < 16; i++) {
    prevValues[i] = 0;
}

function msg_int(n) {
    if (n < 1 || n > 16) {
        return;
    }
    
    lastStart = 0;
    lastEnd = n - 1;
    
    outlet(0, "length", n);
    outlet(0, "offset", 0);
    
    outputPattern(lastStart, lastEnd);
}

function list() {
    // Handle list input
    var args = arrayfromargs(arguments);
    
    if (args.length != 2) {
        return;
    }
    
    var start = args[0] - 1;  
    var end = args[1] - 1;    
    
    if (start < 0 || end > 15 || start > end) {
        return;
    }
    
    lastStart = start;
    lastEnd = end;
    
    var length = Math.abs(end - start + 1);
    var offset = args[0];  
    
    outlet(0, "length", length);
    outlet(0, "offset", offset);
    
    outputPattern(lastStart, lastEnd);
}

function step(s) {
    currentStep = s;
    outputPattern(lastStart, lastEnd);
}

function output() {
    outputPattern(lastStart, lastEnd);
}

function state() {
    for (var group = 4; group <= 5; group++) {
        for (var i = 0; i < 8; i++) {
            var value = 0;
            
            var absPos = (group - 4) * 8 + i;
            
            if (absPos >= lastStart && absPos <= lastEnd) {
                if (currentStep > 0 && absPos == (currentStep - 1)) {
                    value = 29;  // Highlight value for step
                } else {
                    value = 10;  // Normal active value
                }
            }
            
            // Force output regardless of previous value
            outlet(0, "led", i, group, value);
            prevValues[absPos] = value;
        }
    }
}

function outputPattern(startIdx, endIdx) {
    for (var group = 4; group <= 5; group++) {
        for (var i = 0; i < 8; i++) {
            var value = 0;
            
            var absPos = (group - 4) * 8 + i;
            
            if (absPos >= startIdx && absPos <= endIdx) {
                if (currentStep > 0 && absPos == (currentStep - 1)) {
                    value = 29;  // Highlight value for step
                } else {
                    value = 10;  // Normal active value
                }
            }
            
            // Only output if value has changed
            if (prevValues[absPos] != value) {
                outlet(0, "led", i, group, value);
                prevValues[absPos] = value;
            }
        }
    }
}
