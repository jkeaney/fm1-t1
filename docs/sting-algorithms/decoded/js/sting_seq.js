
autowatch = 1;
outlets = 1;
inlets = 1;

var sequenceData = new Array(16);
var gateData = new Array(16);
var velocityData = new Array(16);
var octaveData = new Array(16);
var maxSteps = 16;
var maxNotes = 12;
var isDragging = false;
var dragColumn = -1;
var pulseStep = -1;
var pulseStartTime = 0;
var pulseDuration = 200;
var editMode = "notes";
var lastDragStep = -1;
var sequenceOffset = 0;

var animationTask = null;
var redrawInterval = 33;
var animationInterval = 50;
var lastRedrawTime = 0;
var needsRedraw = false;
var animationActive = false;

var dirtySteps = new Array(16);
var dirtyRegions = {
    fullRedraw: true,
    steps: true,
    pulse: false
};

var cachedLayout = {
    width: 0,
    height: 0,
    stepWidth: 0,
    noteHeight: 0,
    valid: false
};

var drawBatch = {
    rects: [],
    colors: [],
    operations: []
};

for (var i = 0; i < 16; i++) {
    sequenceData[i] = 0;
    gateData[i] = 127;
    velocityData[i] = 2;
    octaveData[i] = 0;
    dirtySteps[i] = true;
}

var colors = {
    bg: [0, 0, 0, 0],
    white: [0.9, 0.9, 0.9, 1.0],
    black: [0.2, 0.2, 0.2, 1.0],
    grid: [0.3, 0.3, 0.3, 0.5],
    note: [0.2, 0.8, 0.2, 0.9],
    text: [1.0, 1.0, 1.0, 1.0]
};


var blackKeys = [1, 3, 6, 8, 10];
var blackKeyLookup = {};
for (var i = 0; i < blackKeys.length; i++) {
    blackKeyLookup[blackKeys[i]] = true;
}

function markStepDirty(step) {
    if (step >= 0 && step < 16) {
        dirtySteps[step] = true;
        dirtyRegions.steps = true;
    }
}

function markFullDirty() {
    dirtyRegions.fullRedraw = true;
    dirtyRegions.steps = true;
    for (var i = 0; i < 16; i++) {
        dirtySteps[i] = true;
    }
}

function clearDirtyFlags() {
    dirtyRegions.fullRedraw = false;
    dirtyRegions.steps = false;
    dirtyRegions.pulse = false;
    for (var i = 0; i < 16; i++) {
        dirtySteps[i] = false;
    }
}

function updateLayout() {
    var newWidth = box.rect[2] - box.rect[0];
    var newHeight = box.rect[3] - box.rect[1];
    
    if (cachedLayout.width !== newWidth || cachedLayout.height !== newHeight || !cachedLayout.valid) {
        cachedLayout.width = newWidth;
        cachedLayout.height = newHeight;
        cachedLayout.stepWidth = newWidth / 16;
        cachedLayout.noteHeight = newHeight / 12;
        cachedLayout.valid = true;
        markFullDirty();
    }
}

function scheduleRedraw() {
    if (needsRedraw) return;
    needsRedraw = true;
    
    var currentTime = new Date().getTime();
    var timeSinceLastRedraw = currentTime - lastRedrawTime;
    
    if (timeSinceLastRedraw >= redrawInterval) {
        performRedraw();
    } else {
        if (animationTask) {
            animationTask.cancel();
        }
        animationTask = new Task(performRedraw);
        animationTask.schedule(redrawInterval - timeSinceLastRedraw);
    }
}

function performRedraw() {
    var currentTime = new Date().getTime();
    var wasAnimating = animationActive;
    
    if (pulseStep >= 0) {
        if (currentTime - pulseStartTime > pulseDuration) {
            markStepDirty(pulseStep);
            pulseStep = -1;
            animationActive = false;
        } else {
            animationActive = true;
            dirtyRegions.pulse = true;
        }
    } else {
        animationActive = false;
    }
    
    if (dirtyRegions.fullRedraw || dirtyRegions.steps || dirtyRegions.pulse) {
        mgraphics.redraw();
    }
    
    needsRedraw = false;
    lastRedrawTime = currentTime;
    
    if (animationActive && !wasAnimating) {
        startAnimation();
    } else if (!animationActive && wasAnimating) {
        stopAnimation();
    }
}

function startAnimation() {
    if (animationActive && !animationTask) {
        function animate() {
            if (!animationActive) return;
            performRedraw();
            if (animationActive) {
                animationTask = new Task(animate);
                animationTask.schedule(animationInterval);
            }
        }
        animate();
    }
}

function stopAnimation() {
    if (animationTask) {
        animationTask.cancel();
        animationTask = null;
    }
}

function notifydeleted() {
    animationActive = false;
    
    if (animationTask) {
        animationTask.cancel();
        animationTask = null;
    }
    
    needsRedraw = false;
    isDragging = false;
    dragColumn = -1;
    pulseStep = -1;
    lastDragStep = -1;
    cachedLayout.valid = false;
    clearDirtyFlags();
}

function bang() {
    markFullDirty();
    mgraphics.redraw();
}

function anything() {
    var msg = messagename;
    var changed = false;
    
    if (msg === "whitekeys") {
        colors.white = [arguments[0], arguments[1], arguments[2], arguments[3]];
        changed = true;
    } else if (msg === "blackkeys") {
        colors.black = [arguments[0], arguments[1], arguments[2], arguments[3]];
        changed = true;
    } else if (msg === "grid") {
        colors.grid = [arguments[0], arguments[1], arguments[2], arguments[3]];
        changed = true;
    } else if (msg === "note") {
        colors.note = [arguments[0], arguments[1], arguments[2], arguments[3]];
        changed = true;
    } else if (msg === "textcolor") {
        colors.text = [arguments[0], arguments[1], arguments[2], arguments[3]];
        changed = true;
    } else if (msg === "gate" && arguments.length >= 16) {
        for (var i = 0; i < 16; i++) {
            var val = arguments[i];
            var newVal = (val === 30 || val === 70) ? val : 127;
            if (gateData[i] !== newVal) {
                gateData[i] = newVal;
                markStepDirty(i);
                changed = true;
            }
        }
    } else if (msg === "vel" && arguments.length >= 16) {
        for (var i = 0; i < 16; i++) {
            var val = arguments[i];
            var newVal = (val >= 0 && val <= 2) ? val : 2;
            if (velocityData[i] !== newVal) {
                velocityData[i] = newVal;
                markStepDirty(i);
                changed = true;
            }
        }
    } else if (msg === "octave" && arguments.length >= 16) {
        for (var i = 0; i < 16; i++) {
            var val = arguments[i];
            var newVal = (val >= -1 && val <= 2) ? val : 0;
            if (octaveData[i] !== newVal) {
                octaveData[i] = newVal;
                markStepDirty(i);
                changed = true;
            }
        }
    } else if (msg === "state") {
        var mode = arguments[0];
        if (mode === "notes" || mode === "vel" || mode === "gate" || mode === "octave") {
            if (editMode !== mode) {
                editMode = mode;
                markFullDirty();
                changed = true;
            }
        }
    } else if (msg === "step" && arguments[0] >= 0 && arguments[0] < 16) {
        var oldPulse = pulseStep;
        pulseStep = arguments[0];
        pulseStartTime = new Date().getTime();
        if (oldPulse !== pulseStep) {
            if (oldPulse >= 0) markStepDirty(oldPulse);
            markStepDirty(pulseStep);
            dirtyRegions.pulse = true;
        }
        startAnimation();
        changed = true;
    } else if (msg === "offset") {
        var newOffset = ((-arguments[0] % 16) + 16) % 16;
        if (sequenceOffset !== newOffset) {
            sequenceOffset = newOffset;
            markFullDirty();
            changed = true;
        }
    } else if (msg === "sequence" && arguments.length >= 16) {
        for (var i = 0; i < 16; i++) {
            var newVal = Math.max(0, Math.min(11, arguments[i]));
            if (sequenceData[i] !== newVal) {
                sequenceData[i] = newVal;
                markStepDirty(i);
                changed = true;
            }
        }
    } else if (msg === "setstep" && arguments[0] >= 0 && arguments[0] < 16) {
        var stepIdx = arguments[0];
        var newVal = Math.max(0, Math.min(11, arguments[1]));
        if (sequenceData[stepIdx] !== newVal) {
            sequenceData[stepIdx] = newVal;
            markStepDirty(stepIdx);
            changed = true;
        }
    } else if (msg === "clear") {
        for (var i = 0; i < 16; i++) {
            sequenceData[i] = 0;
            gateData[i] = 127;
            velocityData[i] = 2;
            octaveData[i] = 0;
        }
        markFullDirty();
        changed = true;
    }
    
    if (changed) {
        scheduleRedraw();
    }
}

function paint() {
    updateLayout();
    
    with (mgraphics) {
        if (dirtyRegions.fullRedraw) {
            set_source_rgba(colors.bg[0], colors.bg[1], colors.bg[2], colors.bg[3]);
            rectangle(0, 0, cachedLayout.width, cachedLayout.height);
            fill();
        }
        
        if (editMode === "octave") {
            drawOctaveViewOptimized();
        } else {
            drawSequencerViewOptimized();
        }
    }
    
    clearDirtyFlags();
}

function drawSequencerViewOptimized() {
    with (mgraphics) {
        for (var note = 0; note < 12; note++) {
            var y = (11 - note) * cachedLayout.noteHeight;
            var isBlack = blackKeyLookup[note];
            
            if (isBlack) {
                set_source_rgba(colors.black[0], colors.black[1], colors.black[2], colors.black[3]);
            } else {
                set_source_rgba(colors.white[0], colors.white[1], colors.white[2], colors.white[3]);
            }
            
            rectangle(0, y, cachedLayout.width, cachedLayout.noteHeight);
            fill();
        }
        
        var currentTime = new Date().getTime();
        var batchRects = [];
        var batchColors = [];
        
        for (var step = 0; step < 16; step++) {
            var dataIndex = (step + sequenceOffset) % 16;
            var noteValue = sequenceData[dataIndex];
            var gateValue = gateData[dataIndex];
            var velocityValue = velocityData[dataIndex];
            var x = step * cachedLayout.stepWidth;
            var y = (11 - noteValue) * cachedLayout.noteHeight;
            
            var gateWidth = gateValue === 30 ? 0.33 : gateValue === 70 ? 0.66 : 1.0;
            
            if (pulseStep === step) {
                var elapsed = currentTime - pulseStartTime;
                if (elapsed < pulseDuration) {
                    var blinkCycle = Math.floor(elapsed / 100);
                    if (blinkCycle % 2 === 0) {
                        set_source_rgba(colors.note[0], colors.note[1], colors.note[2], colors.note[3]);
                        var pulseWidth = (cachedLayout.stepWidth - 4) * gateWidth + 4;
                        drawRoundedRect(x, y, pulseWidth, cachedLayout.noteHeight, 6);
                        fill();
                    }
                }
            }
            
            var margin = 2;
            var rectX = x + margin;
            var rectY = y + margin;
            var rectWidth = (cachedLayout.stepWidth - 4) * gateWidth;
            var rectHeight = cachedLayout.noteHeight - 4;
            
            if (velocityValue === 0) {
                set_source_rgba(colors.note[0], colors.note[1], colors.note[2], colors.note[3] * 0.5);
                set_line_width(1);
                drawRoundedRect(rectX, rectY, rectWidth, rectHeight, 4);
                stroke();
            } else {
                var alpha = velocityValue === 1 ? colors.note[3] * 0.5 : colors.note[3];
                set_source_rgba(colors.note[0], colors.note[1], colors.note[2], alpha);
                drawRoundedRect(rectX, rectY, rectWidth, rectHeight, 4);
                fill();
            }
        }
        
        set_source_rgba(colors.grid[0], colors.grid[1], colors.grid[2], colors.grid[3]);
        set_line_width(1);
        new_path();
        for (var step = 0; step < 16; step++) {
            var x = step * cachedLayout.stepWidth;
            move_to(x, 0);
            line_to(x, cachedLayout.height);
            move_to(x + cachedLayout.stepWidth, 0);
            line_to(x + cachedLayout.stepWidth, cachedLayout.height);
        }
        stroke();
    }
}

function drawOctaveViewOptimized() {
    with (mgraphics) {
        set_source_rgba(colors.grid[0], colors.grid[1], colors.grid[2], colors.grid[3]);
        set_line_width(0.5);
        new_path();
        for (var step = 0; step <= 16; step++) {
            var x = step * cachedLayout.stepWidth;
            move_to(x, 0);
            line_to(x, cachedLayout.height);
        }
        stroke();
        
        select_font_face("Avenir");
        set_font_size(18);
        
        for (var step = 0; step < 16; step++) {
            var dataIndex = (step + sequenceOffset) % 16;
            var octave = octaveData[dataIndex];
            var octaveText = octave > 0 ? "+" + octave : octave.toString();
            
            var x = step * cachedLayout.stepWidth;
            
            if (pulseStep === step) {
                set_source_rgba(colors.note[0], colors.note[1], colors.note[2], colors.note[3]);
            } else {
                set_source_rgba(colors.text[0], colors.text[1], colors.text[2], colors.text[3]);
            }
            
            var textExtents = text_measure(octaveText);
            move_to(x + cachedLayout.stepWidth/2 - textExtents[0]/2, 
                   cachedLayout.height/2 + textExtents[1]/2);
            show_text(octaveText);
        }
    }
}

function drawRoundedRect(x, y, w, h, r) {
    with (mgraphics) {
        new_path();
        move_to(x + r, y);
        line_to(x + w - r, y);
        curve_to(x + w, y, x + w, y + r, x + w, y + r);
        line_to(x + w, y + h - r);
        curve_to(x + w, y + h, x + w - r, y + h, x + w - r, y + h);
        line_to(x + r, y + h);
        curve_to(x, y + h, x, y + h - r, x, y + h - r);
        line_to(x, y + r);
        curve_to(x, y, x + r, y, x + r, y);
        close_path();
    }
}

function handleClick(x, y) {
    var stepClicked = Math.floor(x / cachedLayout.stepWidth);
    if (stepClicked < 0 || stepClicked >= 16) return;
    
    var dataIndex = (stepClicked + sequenceOffset) % 16;
    
    if (editMode === "notes") {
        var noteClicked = 11 - Math.floor(y / cachedLayout.noteHeight);
        if (noteClicked >= 0 && noteClicked < 12) {
            if (sequenceData[dataIndex] !== noteClicked) {
                sequenceData[dataIndex] = noteClicked;
                markStepDirty(stepClicked);
                outlet(0, "sequence", sequenceData);
                outlet(0, "single", stepClicked + 1, noteClicked);
                scheduleRedraw();
            }
        }
    }
    
    isDragging = true;
    dragColumn = stepClicked;
    lastDragStep = stepClicked;
}

function handleDrag(x, y) {
    if (!isDragging || dragColumn < 0) return;
    
    var dataIndex = (dragColumn + sequenceOffset) % 16;
    var yPercent = 1.0 - (y / cachedLayout.height);
    
    if (editMode === "notes") {
        var currentStep = Math.floor(x / cachedLayout.stepWidth);
        var noteClicked = 11 - Math.floor(y / cachedLayout.noteHeight);
        
        if (currentStep >= 0 && currentStep < 16 && noteClicked >= 0 && noteClicked < 12) {
            var startStep = Math.min(lastDragStep, currentStep);
            var endStep = Math.max(lastDragStep, currentStep);
            var changed = false;
            
            for (var step = startStep; step <= endStep; step++) {
                var idx = (step + sequenceOffset) % 16;
                if (sequenceData[idx] !== noteClicked) {
                    sequenceData[idx] = noteClicked;
                    markStepDirty(step);
                    outlet(0, "single", step + 1, noteClicked);
                    changed = true;
                }
            }
            
            if (changed) {
                lastDragStep = currentStep;
                outlet(0, "sequence", sequenceData);
                scheduleRedraw();
            }
        }
    } else if (editMode === "vel") {
        var newVel = yPercent < 0.33 ? 0 : yPercent < 0.66 ? 1 : 2;
        if (velocityData[dataIndex] !== newVel) {
            velocityData[dataIndex] = newVel;
            markStepDirty(dragColumn);
            outlet(0, "vel", velocityData);
            scheduleRedraw();
        }
    } else if (editMode === "gate") {
        var xPercent = (x - (dragColumn * cachedLayout.stepWidth)) / cachedLayout.stepWidth;
        var newGate = xPercent < 0.33 ? 30 : xPercent < 0.66 ? 70 : 127;
        if (gateData[dataIndex] !== newGate) {
            gateData[dataIndex] = newGate;
            markStepDirty(dragColumn);
            outlet(0, "gate", gateData);
            scheduleRedraw();
        }
    } else if (editMode === "octave") {
        var newOctave = yPercent < 0.25 ? -1 : yPercent < 0.5 ? 0 : yPercent < 0.75 ? 1 : 2;
        if (octaveData[dataIndex] !== newOctave) {
            octaveData[dataIndex] = newOctave;
            markStepDirty(dragColumn);
            outlet(0, "octave", octaveData);
            scheduleRedraw();
        }
    }
}

function onclick(x, y) { handleClick(x, y); }
function ondrag(x, y) { handleDrag(x, y); }
function onidlemouse() { isDragging = false; dragColumn = -1; lastDragStep = -1; }

function onresize(w, h) {
    cachedLayout.valid = false;
    markFullDirty();
    scheduleRedraw();
}

function setup() {
    mgraphics.init();
    mgraphics.relative_coords = 0;
    mgraphics.autofill = 0;
}

setup();
