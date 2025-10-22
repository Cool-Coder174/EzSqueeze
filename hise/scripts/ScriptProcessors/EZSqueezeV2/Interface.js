// Frontend size per wireframe spec
Content.makeFrontInterface(900, 650);

// Hide legacy UIData components (Panel + old knobs) to avoid duplicates
const legacyIds = ["Panel1", "Knob1", "Knob2", "Knob3", "Knob4", "Knob5"]; 
for (local i = 0; i < legacyIds.length; i++) {
    local c = Content.getComponent(legacyIds[i]);
    if (c) c.set("visible", false);
}

// Convenience for filmstrip knobs
inline function createFilmstripKnob(id, x, y, size, processorId, parameterId, min, max, step)
{
    local k = Content.addKnob(id, x, y);
    k.set("width", size);
    k.set("height", size);
    k.set("filmstripImage", "{PROJECT_FOLDER}assets/ui/knobs/orangeKnob.png");
    k.set("numStrips", 60);
    k.set("scaleFactor", 1.0);
    k.set("showValuePopup", "Below");
    if (processorId && parameterId)
    {
        k.set("processorId", processorId);
        k.set("parameterId", parameterId);
    }
    k.set("min", min);
    k.set("max", max);
    k.set("stepSize", step);
    return k;
}

// Labels helper
inline function createLabel(id, x, y, text)
{
    local l = Content.addLabel(id, x, y);
    l.set("text", text);
    l.set("editable", false);
    l.set("fontSize", 12);
    l.set("textColour", 0xFFE0E0E0);
    return l;
}

// Core compression controls (wired to HISE Dynamics1)
// Positions approximate the design; adjust later as needed
createLabel("lblThreshold", 260, 120, "THRESHOLD");
createLabel("lblRatio", 380, 120, "RATIO");
createLabel("lblKnee", 500, 120, "KNEE");
createLabel("lblAttack", 620, 120, "ATTACK");
createLabel("lblRelease", 740, 120, "RELEASE");

// Threshold (-60..0 dB)
createFilmstripKnob("thresKnob", 260, 140, 100, "Dynamics1", "CompressorThreshold", -60.0, 0.0, 0.1);
// Ratio (1..32)
createFilmstripKnob("ratioKnob", 380, 140, 100, "Dynamics1", "CompressorRatio", 1.0, 32.0, 0.1);
// Knee selector (placeholder)
local knee = Content.addComboBox("knee", 500, 160);
knee.set("width", 80);
knee.set("height", 24);
knee.set("items", "Hard;Medium;Soft");
knee.set("tooltip", "Transition curve at threshold");
// Attack (0.1..100 ms)
createFilmstripKnob("attackKnob", 620, 140, 90, "Dynamics1", "CompressorAttack", 0.1, 100.0, 0.1);
// Release (10..300 ms) per current HISE Dynamics range
createFilmstripKnob("releaseKnob", 740, 140, 90, "Dynamics1", "CompressorRelease", 10.0, 300.0, 0.1);

// Character section
createLabel("lblVibe", 260, 300, "VIBE");
createLabel("lblDual", 400, 300, "DUAL STACK");
createLabel("lblImpedance", 500, 300, "CHARACTER");
createLabel("lblCloudGain", 640, 300, "CLOUD GAIN");
createLabel("lblMix", 750, 300, "MIX");

// Vibe wheel placeholder
local vibe = Content.addKnob("vibeWheel", 260, 320);
vibe.set("width", 120);
vibe.set("height", 120);
vibe.set("filmstripImage", "{PROJECT_FOLDER}assets/ui/knobs/orangeKnob2.png");
vibe.set("numStrips", 60);
vibe.set("scaleFactor", 1.0);
vibe.set("tooltip", "Blend from transparent to colored");

// Dual-stage toggle (placeholder)
local dual = Content.addToggleButton("dualStack", 400, 340);
dual.set("text", "DUAL");

// Impedance selector (placeholder)
local imp = Content.addComboBox("impedance", 500, 340);
imp.set("width", 110);
imp.set("items", "Silicon;Tube;Transformer");

// Cloud Gain (0..30 dB) - placeholder, not wired yet
createFilmstripKnob("cloudGain", 640, 320, 80, undefined, undefined, 0.0, 30.0, 0.1);

// Mix (0..100 %) - placeholder for parallel blend
createFilmstripKnob("mix", 750, 320, 80, undefined, undefined, 0.0, 100.0, 0.1);

// Advanced panel (placeholders)
createLabel("lblAdvanced", 220, 480, "ADVANCED");

createLabel("lblLookahead", 240, 500, "LOOKAHEAD");
createFilmstripKnob("lookahead", 240, 520, 60, undefined, undefined, 0.0, 10.0, 0.1);

createLabel("lblLink", 320, 500, "LINK");
createFilmstripKnob("stereoLink", 320, 520, 60, undefined, undefined, 0.0, 100.0, 1.0);

local msMode = Content.addToggleButton("msMode", 400, 520);
msMode.set("text", "M/S");

createLabel("lblScHpf", 460, 500, "SC HPF");
createFilmstripKnob("scHpf", 460, 520, 60, undefined, undefined, 20.0, 400.0, 1.0);

createLabel("lblScLpf", 540, 500, "SC LPF");
createFilmstripKnob("scLpf", 540, 520, 60, undefined, undefined, 4000.0, 16000.0, 10.0);

local osSel = Content.addComboBox("oversampling", 620, 520);
osSel.set("width", 70);
osSel.set("items", "Off;2×;4×;8×");

local autoRel = Content.addToggleButton("autoRelease", 700, 520);
autoRel.set("text", "AUTO REL");

local autoMk = Content.addToggleButton("autoMakeup", 800, 520);
autoMk.set("text", "AUTO MK");

// Event callbacks (stubs)
function onNoteOn() {}

function onNoteOff() {}

function onController() {}

function onTimer() {}

function onControl(number, value)
{
    // Placeholder for future parameter mapping (knee, mix, advanced controls)
}
 