
autowatch = 1;
autosketch = 1;

mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;

var lineColor = [0, 0, 0, 1];
var bgColor = [0, 0, 0, 0];

var smileLevel = 8;
var pupilSize = 1;

var isBlinking = false;
var isShaking = false;
var animStartTime = 0;
var animDuration = 150;
var shakeOffset = {x: 0, y: 0};

var randomAnimType = "none";
var randomAnimStartTime = 0;
var randomAnimDuration = 150;
var leftPupilOffset = {x: 0, y: 0};
var rightPupilOffset = {x: 0, y: 0};
var pupilFrameCount = 0;

var cachedDimensions = null;
var needsRedraw = true;
var currentAnimTask = null;
var currentRandomTask = null;

var maxAnimationFrames = 1000;
var currentFrameCount = 0;
var randomFrameCount = 0;

function updateCachedDimensions() {
    var width = box.rect[2] - box.rect[0];
    var height = box.rect[3] - box.rect[1];
    
    if (cachedDimensions) {
        cachedDimensions.width = width;
        cachedDimensions.height = height;
        cachedDimensions.centerX = width / 2;
        cachedDimensions.centerY = height / 2;
        cachedDimensions.radius = Math.min(width, height) * 0.35;
        cachedDimensions.eyeSize = Math.min(width, height) * 0.35 * 0.15;
        cachedDimensions.maxPupilRadius = Math.min(width, height) * 0.35 * 0.13;
        cachedDimensions.minPupilRadius = Math.min(width, height) * 0.35 * 0.02;
        cachedDimensions.mouthWidth = Math.min(width, height) * 0.35;
        cachedDimensions.eyeY = height / 2 - Math.min(width, height) * 0.35 * 0.3;
        cachedDimensions.leftEyeX = width / 2 - Math.min(width, height) * 0.35 * 0.35;
        cachedDimensions.rightEyeX = width / 2 + Math.min(width, height) * 0.35 * 0.35;
        cachedDimensions.mouthY = height / 2 + Math.min(width, height) * 0.35 * 0.25 + 5;
    } else {
        cachedDimensions = {
            width: width,
            height: height,
            centerX: width / 2,
            centerY: height / 2,
            radius: Math.min(width, height) * 0.35,
            eyeSize: Math.min(width, height) * 0.35 * 0.15,
            maxPupilRadius: Math.min(width, height) * 0.35 * 0.13,
            minPupilRadius: Math.min(width, height) * 0.35 * 0.02,
            mouthWidth: Math.min(width, height) * 0.35,
            eyeY: height / 2 - Math.min(width, height) * 0.35 * 0.3,
            leftEyeX: width / 2 - Math.min(width, height) * 0.35 * 0.35,
            rightEyeX: width / 2 + Math.min(width, height) * 0.35 * 0.35,
            mouthY: height / 2 + Math.min(width, height) * 0.35 * 0.25 + 5
        };
    }
}

function cleanupTask(task) {
    if (task) {
        try {
            task.cancel();
            if (task.freepeer) {
                task.freepeer();
            }
        } catch (e) {
            post("Task cleanup error: " + e);
        }
        return null;
    }
    return null;
}

function resetAnimationState() {
    isBlinking = false;
    isShaking = false;
    shakeOffset.x = 0;
    shakeOffset.y = 0;
    currentFrameCount = 0;
    randomFrameCount = 0;
    pupilFrameCount = 0;
    randomAnimType = "none";
    leftPupilOffset.x = 0;
    leftPupilOffset.y = 0;
    rightPupilOffset.x = 0;
    rightPupilOffset.y = 0;
}

function density(val) {
    val = Math.max(1, Math.min(16, val));
    if (smileLevel !== val) {
        smileLevel = val;
        needsRedraw = true;
        mgraphics.redraw();
    }
}

function type(val) {
    val = Math.max(1, Math.min(16, val));
    if (pupilSize !== val) {
        pupilSize = val;
        needsRedraw = true;
        mgraphics.redraw();
    }
}

function rgba(r, g, b, a) {
    if (lineColor[0] !== r || lineColor[1] !== g || lineColor[2] !== b || lineColor[3] !== a) {
        lineColor = [r, g, b, a];
        needsRedraw = true;
        mgraphics.redraw();
    }
}

function onclick(x, y, but, cmd, shift, capslock, option, ctrl) {
    if (!cachedDimensions) updateCachedDimensions();
    
    var dx = x - cachedDimensions.centerX;
    var dy = y - cachedDimensions.centerY;
    var distance = Math.sqrt(dx * dx + dy * dy);
    
    if (distance <= cachedDimensions.radius) {
        startBlinkShake();
        outlet(0, "bang");
    }
}

function click(val) {
    if (val === 1) {
        startBlinkShake();
        outlet(0, "bang");
    }
}

function startBlinkShake() {
    currentAnimTask = cleanupTask(currentAnimTask);
    currentRandomTask = cleanupTask(currentRandomTask);
    
    resetAnimationState();
    
    isBlinking = true;
    isShaking = true;
    animStartTime = new Date().getTime();
    needsRedraw = true;
    mgraphics.redraw();
    
    try {
        if (!currentAnimTask) {
            currentAnimTask = new Task(animationTick, this);
        }
        currentAnimTask.schedule(33);
    } catch (e) {
        post("Animation task creation error: " + e);
        currentAnimTask = null;
        resetAnimationState();
    }
}

function animationTick() {
    var currentTime = new Date().getTime();
    var elapsed = currentTime - animStartTime;
    
    currentFrameCount++;
    
    if (elapsed < animDuration && currentFrameCount < maxAnimationFrames && isBlinking && isShaking) {
        var newShakeX = (Math.random() - 0.5) * 14;
        var newShakeY = (Math.random() - 0.5) * 14;
        
        shakeOffset.x = newShakeX;
        shakeOffset.y = newShakeY;
        needsRedraw = true;
        mgraphics.redraw();
        
        if (currentAnimTask) {
            currentAnimTask.schedule(33);
        }
    } else {
        isBlinking = false;
        isShaking = false;
        shakeOffset.x = 0;
        shakeOffset.y = 0;
        currentFrameCount = 0;
        currentAnimTask = null;
        needsRedraw = true;
        mgraphics.redraw();
    }
}

function randomAnimationTick() {
    pupilFrameCount++;
    randomFrameCount++;
    
    if (pupilFrameCount <= 6 && randomFrameCount < maxAnimationFrames && randomAnimType !== "none") {
        if (!cachedDimensions) updateCachedDimensions();
        
        var progress = pupilFrameCount / 6;
        var maxPupilMovement, targetX, targetY, smoothness;
        
        switch (randomAnimType) {
            case "left_pupil":
                maxPupilMovement = cachedDimensions.radius * 0.05;
                targetX = Math.sin(progress * Math.PI * 4) * maxPupilMovement;
                targetY = Math.cos(progress * Math.PI * 3) * maxPupilMovement;
                
                smoothness = 0.2;
                leftPupilOffset.x = leftPupilOffset.x + (targetX - leftPupilOffset.x) * smoothness;
                leftPupilOffset.y = leftPupilOffset.y + (targetY - leftPupilOffset.y) * smoothness;
                rightPupilOffset.x = rightPupilOffset.x + (0 - rightPupilOffset.x) * smoothness;
                rightPupilOffset.y = rightPupilOffset.y + (0 - rightPupilOffset.y) * smoothness;
                break;
                
            case "right_pupil":
                maxPupilMovement = cachedDimensions.radius * 0.05;
                targetX = Math.sin(progress * Math.PI * 4) * maxPupilMovement;
                targetY = Math.cos(progress * Math.PI * 3) * maxPupilMovement;
                
                smoothness = 0.2;
                rightPupilOffset.x = rightPupilOffset.x + (targetX - rightPupilOffset.x) * smoothness;
                rightPupilOffset.y = rightPupilOffset.y + (targetY - rightPupilOffset.y) * smoothness;
                leftPupilOffset.x = leftPupilOffset.x + (0 - leftPupilOffset.x) * smoothness;
                leftPupilOffset.y = leftPupilOffset.y + (0 - leftPupilOffset.y) * smoothness;
                break;
                
            case "both_pupils":
                maxPupilMovement = cachedDimensions.radius * 0.06;
                targetX = Math.sin(progress * Math.PI * 3) * maxPupilMovement;
                targetY = Math.cos(progress * Math.PI * 4) * maxPupilMovement;
                
                smoothness = 0.15;
                leftPupilOffset.x = leftPupilOffset.x + (targetX - leftPupilOffset.x) * smoothness;
                leftPupilOffset.y = leftPupilOffset.y + (targetY - leftPupilOffset.y) * smoothness;
                rightPupilOffset.x = rightPupilOffset.x + (targetX - rightPupilOffset.x) * smoothness;
                rightPupilOffset.y = rightPupilOffset.y + (targetY - rightPupilOffset.y) * smoothness;
                break;
        }
        
        needsRedraw = true;
        mgraphics.redraw();
        
        if (currentRandomTask) {
            currentRandomTask.schedule(33);
        }
    } else {
        randomAnimType = "none";
        leftPupilOffset.x = 0;
        leftPupilOffset.y = 0;
        rightPupilOffset.x = 0;
        rightPupilOffset.y = 0;
        pupilFrameCount = 0;
        randomFrameCount = 0;
        currentRandomTask = null;
        needsRedraw = true;
        mgraphics.redraw();
    }
}

function animate(duration) {
    currentRandomTask = cleanupTask(currentRandomTask);
    
    var animTypes = ["left_pupil", "right_pupil", "both_pupils"];
    randomAnimType = animTypes[Math.floor(Math.random() * animTypes.length)];
    
    pupilFrameCount = 0;
    randomFrameCount = 0;
    
    try {
        if (!currentRandomTask) {
            currentRandomTask = new Task(randomAnimationTick, this);
        }
        currentRandomTask.schedule(33);
    } catch (e) {
        post("Random animation task creation error: " + e);
        currentRandomTask = null;
        randomAnimType = "none";
        leftPupilOffset.x = 0;
        leftPupilOffset.y = 0;
        rightPupilOffset.x = 0;
        rightPupilOffset.y = 0;
    }
}

function paint() {
    if (!cachedDimensions) {
        updateCachedDimensions();
    }
    
    var d = cachedDimensions;
    
    with (mgraphics) {
        set_source_rgba(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        rectangle(0, 0, d.width, d.height);
        fill();
        
        set_source_rgba(lineColor[0], lineColor[1], lineColor[2], lineColor[3]);
        set_line_width(1);
        set_line_cap("round");
        set_line_join("round");
        
        var centerX = d.centerX + shakeOffset.x;
        var centerY = d.centerY + shakeOffset.y;
        
        arc(centerX, centerY, d.radius, 0, Math.PI * 2);
        stroke();
        
        var pupilRadius = d.minPupilRadius + ((pupilSize - 1) / 15) * (d.maxPupilRadius - d.minPupilRadius);
        
        if (isBlinking) {
            move_to(d.leftEyeX - d.eyeSize, d.eyeY);
            line_to(d.leftEyeX + d.eyeSize, d.eyeY);
            stroke();
            
            move_to(d.rightEyeX - d.eyeSize, d.eyeY);
            line_to(d.rightEyeX + d.eyeSize, d.eyeY);
            stroke();
        } else {
            save();
            translate(d.leftEyeX, d.eyeY);
            scale(1, 1.5);
            arc(0, 0, d.eyeSize, 0, Math.PI * 2);
            restore();
            stroke();
            
            save();
            translate(d.rightEyeX, d.eyeY);
            scale(1, 1.5);
            arc(0, 0, d.eyeSize, 0, Math.PI * 2);
            restore();
            stroke();
            
            save();
            translate(d.leftEyeX + leftPupilOffset.x, d.eyeY + leftPupilOffset.y);
            scale(1, 1.5);
            arc(0, 0, pupilRadius, 0, Math.PI * 2);
            fill();
            restore();
            
            save();
            translate(d.rightEyeX + rightPupilOffset.x, d.eyeY + rightPupilOffset.y);
            scale(1, 1.5);
            arc(0, 0, pupilRadius, 0, Math.PI * 2);
            fill();
            restore();
        }
        
        var smileAmount = (smileLevel - 1) / 15;
        var curveHeight = d.radius * 0.35 * smileAmount;
        
        move_to(centerX - d.mouthWidth/2, d.mouthY);
        
        if (smileAmount < 0.05) {
            line_to(centerX + d.mouthWidth/2, d.mouthY);
        } else {
            var cp1x = centerX - d.mouthWidth/3;
            var cp1y = d.mouthY + curveHeight;
            var cp2x = centerX + d.mouthWidth/3;
            var cp2y = d.mouthY + curveHeight;
            
            curve_to(cp1x, cp1y, cp2x, cp2y, centerX + d.mouthWidth/2, d.mouthY);
        }
        stroke();
    }
    
    needsRedraw = false;
}

function bang() {
    needsRedraw = true;
    mgraphics.redraw();
}

function onresize(w, h) {
    updateCachedDimensions();
    needsRedraw = true;
}

function notifydeleted() {
    currentAnimTask = cleanupTask(currentAnimTask);
    currentRandomTask = cleanupTask(currentRandomTask);
    resetAnimationState();
    cachedDimensions = null;
}
