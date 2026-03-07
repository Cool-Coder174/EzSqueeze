Content.makeFrontInterface(900, 650);

// ════════════════════════════════════════════════════════════════
// PROCESSOR REFERENCE
// ════════════════════════════════════════════════════════════════
const var Compressor = Synth.getEffect("Dynamics1");

// ════════════════════════════════════════════════════════════════
// COLOUR PALETTE
// ════════════════════════════════════════════════════════════════
const var COL_BG           = 0xFF1A1A1D;
const var COL_PANEL        = 0xFF252528;
const var COL_PANEL_LT     = 0xFF2A2A2E;
const var COL_PANEL_BORDER = 0x30EF932C;
const var COL_ORANGE       = 0xFFEF932C;
const var COL_ORANGE_LT    = 0xFFF1B233;
const var COL_ORANGE_DK    = 0xFFDA761E;
const var COL_ACCENT       = 0xFFEE811C;
const var COL_TEXT          = 0xFFE0E0E0;
const var COL_TEXT_DIM      = 0xFF909090;
const var COL_KNOB_BG      = 0xFF3A3A3E;
const var COL_KNOB_FACE    = 0xFF2A2A2E;
const var COL_KNOB_BORDER  = 0xFF4A4A4E;
const var COL_METER_BG     = 0xFF222225;
const var COL_GREEN        = 0xFF4CAF50;
const var COL_YELLOW       = 0xFFFFC107;
const var COL_RED          = 0xFFF44336;

// ════════════════════════════════════════════════════════════════
// ARC / ANGLE CONSTANTS
// ════════════════════════════════════════════════════════════════
const var ARC_START = 2.356;    // 3π/4  screen coords  (7:30)
const var ARC_END   = 7.069;    // 9π/4  screen coords  (4:30)
const var ARC_RANGE = 4.713;    // sweep  = 3π/2
const var ANG_START = -2.356;   // −3π/4 from 12-o-clock CW
const var ANG_RANGE =  4.712;   // total angular range

// ════════════════════════════════════════════════════════════════
// MUTABLE STATE
// ════════════════════════════════════════════════════════════════
reg grValue   = 0.0;
reg inLevelL  = 0.0;
reg inLevelR  = 0.0;
reg outLevelL = 0.0;
reg outLevelR = 0.0;
reg tooltipText = "";
reg abState       = 0;
reg isLoadingSlot = 0;
reg grPeak        = 0.0;
reg grReadout     = 0.0;

const var NUM_AB   = 19;
const var abSlotA  = [];
const var abSlotB  = [];
const var undoState = [];
const var redoState = [];

var _i = 0;
while (_i < NUM_AB)
{
    abSlotA[_i]  = 0.0;
    abSlotB[_i]  = 0.0;
    undoState[_i] = 0.0;
    redoState[_i] = 0.0;
    _i = _i + 1;
}

// ════════════════════════════════════════════════════════════════
// LOCAL LOOK-AND-FEEL
// ════════════════════════════════════════════════════════════════
const var laf = Content.createLocalLookAndFeel();

// ── drawRotarySlider ─────────────────────────────────────────
laf.registerFunction("drawRotarySlider", function(g, obj)
{
    var a = obj.area;
    var w = a[2];
    var h = a[3];
    var size = Math.min(w, h);
    var cx = w * 0.5;
    var cy = h * 0.5;
    var outerR = size * 0.44;
    var innerR = outerR * 0.68;

    // Drop shadow
    g.setColour(0x28000000);
    g.fillEllipse([cx - outerR + 1, cy - outerR + 3,
                   outerR * 2, outerR * 2]);

    // Outer track ring
    g.setColour(COL_KNOB_BG);
    g.fillEllipse([cx - outerR, cy - outerR, outerR * 2, outerR * 2]);
    g.setColour(COL_KNOB_BORDER);
    g.drawEllipse([cx - outerR, cy - outerR, outerR * 2, outerR * 2], 1.0);

    // ── Arc track (dots along the ring) ──
    var arcR    = (outerR + innerR) * 0.52;
    var nDots   = Math.max(20, Math.round(size * 0.4));
    var dotSz   = Math.max(2.0, size * 0.028);
    var stepAng = ARC_RANGE / nDots;

    // Background arc dots (dark)
    g.setColour(0xFF555560);
    var si = 0;
    while (si <= nDots)
    {
        var angle = ARC_START + si * stepAng;
        var dx = cx + arcR * Math.cos(angle);
        var dy = cy + arcR * Math.sin(angle);
        g.fillEllipse([dx - dotSz * 0.5, dy - dotSz * 0.5, dotSz, dotSz]);
        si = si + 1;
    }

    // Colour based on interaction state
    var col = COL_ORANGE;
    if (obj.hover)   col = COL_ORANGE_LT;
    if (obj.clicked) col = COL_ACCENT;

    // Value arc dots (orange)
    if (obj.valueNormalized > 0.005)
    {
        var valDots = Math.max(1, Math.round(obj.valueNormalized * nDots));
        g.setColour(col);
        si = 0;
        while (si <= valDots)
        {
            var angle = ARC_START + si * stepAng;
            var dx = cx + arcR * Math.cos(angle);
            var dy = cy + arcR * Math.sin(angle);
            g.fillEllipse([dx - dotSz * 0.5, dy - dotSz * 0.5, dotSz, dotSz]);
            si = si + 1;
        }
    }

    // Inner knob face
    g.setColour(COL_KNOB_FACE);
    g.fillEllipse([cx - innerR, cy - innerR, innerR * 2, innerR * 2]);

    // Subtle highlight (upper-left)
    g.setColour(0x0CFFFFFF);
    g.fillEllipse([cx - innerR * 0.85, cy - innerR * 0.92,
                   innerR * 1.3, innerR * 0.85]);

    // Inner border
    g.setColour(0xFF343438);
    g.drawEllipse([cx - innerR, cy - innerR, innerR * 2, innerR * 2], 0.8);

    // Pointer dot
    var ang   = ANG_START + obj.valueNormalized * ANG_RANGE;
    var dotR  = Math.max(2.5, size * 0.038);
    var dotD  = innerR * 0.62;
    var dotX  = cx + Math.sin(ang) * dotD;
    var dotY  = cy - Math.cos(ang) * dotD;
    g.setColour(col);
    g.fillEllipse([dotX - dotR, dotY - dotR, dotR * 2, dotR * 2]);

    // Hover glow
    if (obj.hover || obj.clicked)
    {
        g.setColour(0x10EF932C);
        g.fillEllipse([cx - outerR, cy - outerR, outerR * 2, outerR * 2]);
    }
});

// ── drawToggleButton ─────────────────────────────────────────
laf.registerFunction("drawToggleButton", function(g, obj)
{
    var a = obj.area;
    var w = a[2];
    var h = a[3];
    var cr = h * 0.5;
    var on = obj.value;

    if (on)
    {
        g.setColour(0x80EF932C);
        g.fillRoundedRectangle([0, 0, w, h], cr);
        g.setColour(COL_ORANGE);
        g.drawRoundedRectangle([0, 0, w, h], cr, 1.5);
    }
    else
    {
        g.setColour(0xFF333336);
        g.fillRoundedRectangle([0, 0, w, h], cr);
        g.setColour(0xFF555558);
        g.drawRoundedRectangle([0, 0, w, h], cr, 1.0);
    }

    if (obj.over)
    {
        g.setColour(on ? 0x18FFFFFF : 0x10FFFFFF);
        g.fillRoundedRectangle([0, 0, w, h], cr);
    }

    g.setFont("Arial Bold", Math.min(11, h * 0.45));
    g.setColour(on ? COL_TEXT : COL_TEXT_DIM);
    g.drawAlignedText(obj.text, [0, 0, w, h], "centred");
});

// ── drawComboBox ─────────────────────────────────────────────
laf.registerFunction("drawComboBox", function(g, obj)
{
    var a = obj.area;
    var w = a[2];
    var h = a[3];

    g.setColour(0xFF333336);
    g.fillRoundedRectangle([0, 0, w, h], 4);
    g.setColour(obj.hover ? COL_ORANGE : 0xFF555558);
    g.drawRoundedRectangle([0, 0, w, h], 4, 1.0);

    g.setFont("Arial", 12);
    g.setColour(obj.hover ? COL_TEXT : COL_TEXT_DIM);
    g.drawAlignedText(obj.text, [8, 0, w - 24, h], "left");

    // Dropdown arrow
    var p = Content.createPath();
    p.startNewSubPath(0.0, 0.0);
    p.lineTo(1.0, 0.0);
    p.lineTo(0.5, 1.0);
    p.closeSubPath();
    g.setColour(COL_TEXT_DIM);
    g.fillPath(p, [w - 16, h * 0.5 - 3, 10, 6]);
});

// ════════════════════════════════════════════════════════════════
// HELPER FUNCTIONS
// ════════════════════════════════════════════════════════════════

inline function drawLevelBar(gfx, bx, bw, totalH, level)
{
    local fillH = level * totalH;
    if (fillH > 1.0)
    {
        local fillY = totalH - fillH;
        local y70   = totalH * 0.3;
        local y90   = totalH * 0.1;

        gfx.setColour(COL_GREEN);
        gfx.fillRect([bx, fillY, bw, fillH]);

        if (fillY < y70)
        {
            gfx.setColour(COL_YELLOW);
            gfx.fillRect([bx, fillY, bw, y70 - fillY]);
        }
        if (fillY < y90)
        {
            gfx.setColour(COL_RED);
            gfx.fillRect([bx, fillY, bw, y90 - fillY]);
        }
    }
};

inline function makeKnob(id, x, y, w, h, lo, hi, step, mid, def, sfx)
{
    local k = Content.addKnob(id, x, y);
    k.set("width",  w);
    k.set("height", h);
    k.set("min", lo);
    k.set("max", hi);
    k.set("stepSize", step);
    if (mid > lo) k.set("middlePosition", mid);
    k.set("defaultValue", def);
    k.set("suffix", sfx);
    k.set("style", "Knob");
    k.set("showValuePopup", "Below");
    k.set("textColour", COL_TEXT);
    k.setLocalLookAndFeel(laf);
    return k;
};

inline function makeLabel(id, x, y, w, text, sz)
{
    local lbl = Content.addLabel(id, x, y);
    lbl.set("width",  w);
    lbl.set("height", 18);
    lbl.set("text",      text);
    lbl.set("fontName",  "Arial");
    lbl.set("fontSize",  sz);
    lbl.set("fontStyle", "Bold");
    lbl.set("textColour", COL_TEXT_DIM);
    lbl.set("alignment", "centred");
    lbl.set("editable",  0);
    return lbl;
};

inline function makeButton(id, x, y, w, h, text, momentary)
{
    local b = Content.addButton(id, x, y);
    b.set("width",  w);
    b.set("height", h);
    b.set("text", text);
    if (momentary) b.set("isMomentary", 1);
    b.setLocalLookAndFeel(laf);
    return b;
};

inline function makeCombo(id, x, y, w, h, items)
{
    local c = Content.addComboBox(id, x, y);
    c.set("width",  w);
    c.set("height", h);
    c.set("items", items);
    c.set("textColour", COL_TEXT);
    c.setLocalLookAndFeel(laf);
    return c;
};

// ════════════════════════════════════════════════════════════════
// BACKGROUND PANEL
// ════════════════════════════════════════════════════════════════
const var pnlBg = Content.addPanel("pnlBg", 0, 0);
pnlBg.set("width",  900);
pnlBg.set("height", 650);
pnlBg.set("allowCallbacks", "No Callbacks");

pnlBg.setPaintRoutine(function(g)
{
    g.fillAll(COL_BG);

    // Header
    g.setColour(COL_PANEL);
    g.fillRoundedRectangle([12, 5, 876, 60], 6);
    g.setColour(COL_PANEL_BORDER);
    g.drawRoundedRectangle([12, 5, 876, 60], 6, 1.0);

    // Metering section
    g.setColour(COL_PANEL);
    g.fillRoundedRectangle([12, 75, 188, 440], 6);
    g.setColour(COL_PANEL_BORDER);
    g.drawRoundedRectangle([12, 75, 188, 440], 6, 1.0);

    // Main controls
    g.setColour(COL_PANEL);
    g.fillRoundedRectangle([210, 75, 678, 280], 6);
    g.setColour(COL_PANEL_BORDER);
    g.drawRoundedRectangle([210, 75, 678, 280], 6, 1.0);

    // Row divider
    g.setColour(0x12FFFFFF);
    g.fillRect([230, 222, 638, 1]);

    // Advanced panel
    g.setColour(COL_PANEL_LT);
    g.fillRoundedRectangle([210, 365, 678, 90], 6);
    g.setColour(COL_PANEL_BORDER);
    g.drawRoundedRectangle([210, 365, 678, 90], 6, 1.0);

    // Footer
    g.setColour(COL_PANEL);
    g.fillRoundedRectangle([12, 540, 876, 100], 6);
    g.setColour(COL_PANEL_BORDER);
    g.drawRoundedRectangle([12, 540, 876, 100], 6, 1.0);

    // ── Header text ──
    g.setFont("Arial Bold", 38);
    g.setColour(COL_ORANGE);
    g.drawText("SQUEEZE", [25, 12, 230, 46]);

    g.setFont("Arial", 12);
    g.setColour(COL_TEXT_DIM);
    g.drawText("Citrus Dynamics", [255, 28, 200, 18]);

    // ── Meter labels ──
    g.setFont("Arial Bold", 10);
    g.setColour(COL_TEXT_DIM);
    g.drawAlignedText("IN",  [24, 478, 60, 16], "centred");
    g.setColour(COL_ORANGE);
    g.drawAlignedText("GR",  [86, 478, 60, 16], "centred");
    g.setColour(COL_TEXT_DIM);
    g.drawAlignedText("OUT", [144, 478, 60, 16], "centred");

    // ── Advanced label ──
    g.setFont("Arial", 9);
    g.setColour(0xFF666666);
    g.drawText("ADVANCED", [218, 368, 60, 14]);

    // ── Version ──
    g.setFont("Arial", 10);
    g.setColour(0xFF505050);
    g.drawAlignedText("EzSqueeze v2.0", [750, 620, 130, 16], "right");
});

// ════════════════════════════════════════════════════════════════
// HEADER CONTROLS
// ════════════════════════════════════════════════════════════════
const var cmbDetector = makeCombo("cmbDetector", 680, 18, 80, 28, "Peak\nRMS");
cmbDetector.setValue(1);
const var btnEcoMode  = makeButton("btnEcoMode", 780, 18, 60, 28, "ECO", 0);

// ════════════════════════════════════════════════════════════════
// METERING — INPUT L/R
// ════════════════════════════════════════════════════════════════
const var pnlInputMeter = Content.addPanel("pnlInputMeter", 28, 90);
pnlInputMeter.set("width",  50);
pnlInputMeter.set("height", 380);
pnlInputMeter.set("allowCallbacks", "No Callbacks");

pnlInputMeter.setPaintRoutine(function(g)
{
    var w = this.getWidth();
    var h = this.getHeight();
    var barW = 20;
    var gap  = 4;
    var lx   = 3;
    var rx   = lx + barW + gap;

    g.setColour(COL_METER_BG);
    g.fillRoundedRectangle([lx, 0, barW, h], 3);
    g.fillRoundedRectangle([rx, 0, barW, h], 3);

    drawLevelBar(g, lx + 1, barW - 2, h, inLevelL);
    drawLevelBar(g, rx + 1, barW - 2, h, inLevelR);

    // Tick marks
    g.setColour(0x15FFFFFF);
    var t = 0;
    while (t <= 4)
    {
        var ty = (t / 4.0) * h;
        g.fillRect([lx, ty, barW, 1]);
        g.fillRect([rx, ty, barW, 1]);
        t = t + 1;
    }

    // Channel labels
    g.setFont("Arial Bold", 9);
    g.setColour(COL_TEXT_DIM);
    g.drawAlignedText("L", [lx, h - 14, barW, 12], "centred");
    g.drawAlignedText("R", [rx, h - 14, barW, 12], "centred");
});

// ════════════════════════════════════════════════════════════════
// METERING — GAIN REDUCTION
// ════════════════════════════════════════════════════════════════
const var pnlGrMeter = Content.addPanel("pnlGrMeter", 90, 90);
pnlGrMeter.set("width",  50);
pnlGrMeter.set("height", 380);
pnlGrMeter.set("allowCallbacks", "No Callbacks");

pnlGrMeter.setPaintRoutine(function(g)
{
    var w = this.getWidth();
    var h = this.getHeight();

    g.setColour(COL_METER_BG);
    g.fillRoundedRectangle([3, 0, w - 6, h], 3);

    // GR bar (top-down, 0…24 dB range)
    var barH = Math.min(Math.abs(grValue) / 24.0 * h, h);
    if (barH > 1.0)
    {
        g.setColour(COL_ORANGE);
        g.fillRoundedRectangle([5, 2, w - 10, barH], 2);

        g.setColour(COL_ORANGE_LT);
        g.fillRect([5, Math.max(0, barH - 2), w - 10, 2]);
    }

    // Peak-hold indicator line
    var peakH = Math.min(Math.abs(grPeak) / 24.0 * h, h);
    if (peakH > 2.0)
    {
        g.setColour(COL_ACCENT);
        g.fillRect([5, peakH - 1, w - 10, 2]);
    }

    // Tick marks
    g.setColour(0x18FFFFFF);
    var t = 0;
    while (t <= 4)
    {
        g.fillRect([3, (t / 4.0) * h, w - 6, 1]);
        t = t + 1;
    }

    // Scale labels
    g.setFont("Arial", 8);
    g.setColour(COL_TEXT_DIM);
    g.drawAlignedText("0",   [0, 2,            w, 10], "centred");
    g.drawAlignedText("-6",  [0, h * 0.25 - 5, w, 10], "centred");
    g.drawAlignedText("-12", [0, h * 0.50 - 5, w, 10], "centred");
    g.drawAlignedText("-18", [0, h * 0.75 - 5, w, 10], "centred");
    g.drawAlignedText("-24", [0, h - 12,       w, 10], "centred");
});

// ════════════════════════════════════════════════════════════════
// METERING — OUTPUT L/R
// ════════════════════════════════════════════════════════════════
const var pnlOutputMeter = Content.addPanel("pnlOutputMeter", 148, 90);
pnlOutputMeter.set("width",  50);
pnlOutputMeter.set("height", 380);
pnlOutputMeter.set("allowCallbacks", "No Callbacks");

pnlOutputMeter.setPaintRoutine(function(g)
{
    var w = this.getWidth();
    var h = this.getHeight();
    var barW = 20;
    var gap  = 4;
    var lx   = 3;
    var rx   = lx + barW + gap;

    g.setColour(COL_METER_BG);
    g.fillRoundedRectangle([lx, 0, barW, h], 3);
    g.fillRoundedRectangle([rx, 0, barW, h], 3);

    drawLevelBar(g, lx + 1, barW - 2, h, outLevelL);
    drawLevelBar(g, rx + 1, barW - 2, h, outLevelR);

    // Tick marks
    g.setColour(0x15FFFFFF);
    var t = 0;
    while (t <= 4)
    {
        var ty = (t / 4.0) * h;
        g.fillRect([lx, ty, barW, 1]);
        g.fillRect([rx, ty, barW, 1]);
        t = t + 1;
    }

    g.setFont("Arial Bold", 9);
    g.setColour(COL_TEXT_DIM);
    g.drawAlignedText("L", [lx, h - 14, barW, 12], "centred");
    g.drawAlignedText("R", [rx, h - 14, barW, 12], "centred");
});

// ════════════════════════════════════════════════════════════════
// MAIN CONTROLS — TOP ROW
// ════════════════════════════════════════════════════════════════
const var knbThreshold = makeKnob("knbThreshold", 240, 95, 100, 100,
                                   -60, 0, 0.1, -24, -12, " dB");
const var knbRatio     = makeKnob("knbRatio", 360, 95, 100, 100,
                                   1, 32, 0.1, 4, 4, " :1");
const var cmbKnee      = makeCombo("cmbKnee", 480, 120, 80, 28,
                                    "Hard\nMedium\nSoft");
cmbKnee.setValue(1);
const var knbAttack    = makeKnob("knbAttack", 580, 95, 100, 100,
                                   0.1, 100, 0.1, 15, 4, " ms");
const var knbRelease   = makeKnob("knbRelease", 700, 95, 100, 100,
                                   5, 500, 1, 100, 300, " ms");

makeLabel("lblThreshold", 240, 198, 100, "THRESHOLD", 10);
makeLabel("lblRatio",     360, 198, 100, "RATIO",     10);
makeLabel("lblKnee",      480, 152, 80,  "KNEE",      10);
makeLabel("lblAttack",    580, 198, 100, "ATTACK",    10);
makeLabel("lblRelease",   700, 198, 100, "RELEASE",   10);

// ════════════════════════════════════════════════════════════════
// MAIN CONTROLS — BOTTOM ROW
// ════════════════════════════════════════════════════════════════
const var knbVibeWheel = makeKnob("knbVibeWheel", 240, 237, 100, 100,
                                   0, 100, 0.1, 0, 0, " %");
const var btnDualStack = makeButton("btnDualStack", 370, 268, 70, 30,
                                     "DUAL", 0);
const var cmbImpedance = makeCombo("cmbImpedance", 460, 270, 100, 28,
                                    "Silicon\nTube\nTransformer");
cmbImpedance.setValue(1);
const var knbCloudGain = makeKnob("knbCloudGain", 580, 242, 90, 90,
                                   0, 30, 0.1, 0, 0, " dB");
const var knbMix       = makeKnob("knbMix", 700, 242, 90, 90,
                                   0, 100, 1, 0, 100, " %");

makeLabel("lblVibe",      235, 340, 110, "VIBE",       10);
makeLabel("lblDualStack", 360, 300, 90,  "DUAL STACK",  9);
makeLabel("lblImpedance", 455, 300, 110, "IMPEDANCE",   9);
makeLabel("lblCloudGain", 575, 335, 100, "CLOUD GAIN", 10);
makeLabel("lblMix",       700, 335, 90,  "MIX",        10);

// ════════════════════════════════════════════════════════════════
// ADVANCED PANEL
// ════════════════════════════════════════════════════════════════
const var knbLookahead   = makeKnob("knbLookahead",  230, 375, 65, 65,
                                     0, 10, 0.1, 0, 0, " ms");
const var knbStereoLink  = makeKnob("knbStereoLink", 310, 375, 65, 65,
                                     0, 100, 1, 0, 100, " %");
const var btnMsMode      = makeButton("btnMsMode",    395, 390, 50, 28,
                                       "M/S", 0);
const var knbScHpf       = makeKnob("knbScHpf", 460, 375, 65, 65,
                                     20, 400, 1, 80, 20, " Hz");
const var knbScLpf       = makeKnob("knbScLpf", 540, 375, 65, 65,
                                     4000, 16000, 100, 8000, 16000, " Hz");
const var cmbOversampling = makeCombo("cmbOversampling", 620, 390, 65, 28,
                                       "1x\n2x\n4x\n8x");
cmbOversampling.setValue(1);
const var btnAutoRelease = makeButton("btnAutoRelease", 700, 390, 60, 28,
                                       "A.REL", 0);
const var btnAutoMakeup  = makeButton("btnAutoMakeup",  780, 390, 60, 28,
                                       "A.MU", 0);

makeLabel("lblLookahead",   220, 442, 85, "LOOK",    9);
makeLabel("lblStereoLink",  300, 442, 85, "LINK",    9);
makeLabel("lblMsMode",      388, 420, 65, "M/S",     9);
makeLabel("lblScHpf",       450, 442, 85, "SC HPF",  9);
makeLabel("lblScLpf",       530, 442, 85, "SC LPF",  9);
makeLabel("lblOversampling", 612, 420, 80, "OVERSMP", 9);
makeLabel("lblAutoRelease", 693, 420, 75, "A.REL",   9);
makeLabel("lblAutoMakeup",  773, 420, 75, "A.MU",    9);

// ════════════════════════════════════════════════════════════════
// FOOTER — PRESET BROWSER & UTILITIES
// ════════════════════════════════════════════════════════════════
const var PRESET_ITEMS = "Default\nGentle Bus\nVocal Squeeze\nDrum Punch\nBass Control\nParallel Pump\nStereo Glue\nTransparent\nHeavy Smash\nCitrus Special";
const var cmbPreset = makeCombo("cmbPreset", 25, 560, 250, 30, PRESET_ITEMS);
cmbPreset.setValue(1);

const var btnAB            = makeButton("btnAB",            300, 562, 50, 28, "A / B", 0);
const var btnUndo          = makeButton("btnUndo",          370, 562, 40, 28, "\u2190", 1);
const var btnRedo          = makeButton("btnRedo",          420, 562, 40, 28, "\u2192", 1);
const var btnAutoSweetSpot = makeButton("btnAutoSweetSpot", 490, 562, 120, 28, "SWEET SPOT", 1);

const var knbMakeup = makeKnob("knbMakeup", 660, 550, 80, 80,
                                0, 24, 0.1, 0, 0, " dB");
makeLabel("lblMakeup", 655, 630, 90, "MAKEUP", 10);

// ════════════════════════════════════════════════════════════════
// TRANSFER CURVE PANEL
// ════════════════════════════════════════════════════════════════
const var pnlCurve = Content.addPanel("pnlCurve", 210, 465);
pnlCurve.set("width",  300);
pnlCurve.set("height", 65);
pnlCurve.set("allowCallbacks", "No Callbacks");

pnlCurve.setPaintRoutine(function(g)
{
    var w = this.getWidth();
    var h = this.getHeight();
    var pad = 4;
    var pw = w - pad * 2;
    var ph = h - pad * 2;

    // Background
    g.setColour(0xFF1E1E21);
    g.fillRoundedRectangle([0, 0, w, h], 4);
    g.setColour(0x18EF932C);
    g.drawRoundedRectangle([0, 0, w, h], 4, 1.0);

    // Grid lines
    g.setColour(0x10FFFFFF);
    g.fillRect([pad, pad + ph * 0.5, pw, 1]);
    g.fillRect([pad + pw * 0.5, pad, 1, ph]);

    // Unity line (diagonal)
    g.setColour(0x18FFFFFF);
    var di = 0;
    while (di < pw)
    {
        g.fillRect([pad + di, pad + ph - di * (ph / pw), 1, 1]);
        di = di + 2;
    }

    // Compute transfer curve from current threshold / ratio
    var thresh = knbThreshold.getValue();
    var ratio  = knbRatio.getValue();
    var dbMin  = -60.0;
    var dbMax  = 0.0;
    var dbRange = dbMax - dbMin;

    g.setColour(COL_ORANGE);
    var prev_x = -1;
    var prev_y = -1;
    var ci = 0;
    while (ci <= pw)
    {
        var inputDb = dbMin + (ci / pw) * dbRange;
        var outputDb = inputDb;

        if (inputDb > thresh)
            outputDb = thresh + (inputDb - thresh) / ratio;

        var nx = pad + ci;
        var ny = pad + ph - ((outputDb - dbMin) / dbRange) * ph;
        ny = Math.max(pad, Math.min(pad + ph, ny));

        if (prev_x >= 0)
            g.fillRect([nx, ny, 2, 1]);
        else
            g.fillRect([nx, ny, 1, 1]);

        prev_x = nx;
        prev_y = ny;
        ci = ci + 1;
    }

    // Threshold marker
    var threshX = pad + ((thresh - dbMin) / dbRange) * pw;
    g.setColour(0x40EF932C);
    g.fillRect([threshX, pad, 1, ph]);

    // Label
    g.setFont("Arial", 8);
    g.setColour(0xFF666666);
    g.drawAlignedText("CURVE", [4, 1, 40, 10], "left");
});

// ════════════════════════════════════════════════════════════════
// GR READOUT PANEL (numeric display)
// ════════════════════════════════════════════════════════════════
const var pnlGrReadout = Content.addPanel("pnlGrReadout", 30, 498);
pnlGrReadout.set("width",  140);
pnlGrReadout.set("height", 14);
pnlGrReadout.set("allowCallbacks", "No Callbacks");

pnlGrReadout.setPaintRoutine(function(g)
{
    var w = this.getWidth();
    var h = this.getHeight();
    var val = Math.round(grPeak * 10) / 10;

    g.setFont("Arial Bold", 10);

    if (val < -0.5)
    {
        g.setColour(COL_ORANGE);
        g.drawAlignedText("GR: " + val + " dB", [0, 0, w, h], "centred");
    }
    else
    {
        g.setColour(0xFF555558);
        g.drawAlignedText("GR: 0.0 dB", [0, 0, w, h], "centred");
    }
});

// ════════════════════════════════════════════════════════════════
// TOOLTIP PANEL
// ════════════════════════════════════════════════════════════════
const var pnlTooltip = Content.addPanel("pnlTooltip", 25, 610);
pnlTooltip.set("width",  260);
pnlTooltip.set("height", 20);
pnlTooltip.set("allowCallbacks", "No Callbacks");

pnlTooltip.setPaintRoutine(function(g)
{
    if (tooltipText != "")
    {
        g.setFont("Arial", 12);
        g.setColour(COL_TEXT_DIM);
        g.drawAlignedText(tooltipText, [0, 0, this.getWidth(), this.getHeight()], "left");
    }
});

// ════════════════════════════════════════════════════════════════
// COMBO-BOX ITEM NAMES (for tooltip display; values are 1-based)
// ════════════════════════════════════════════════════════════════
const var KNEE_NAMES = ["Hard", "Medium", "Soft"];
const var IMP_NAMES  = ["Silicon", "Tube", "Transformer"];
const var OS_NAMES   = ["1x", "2x", "4x", "8x"];
const var DET_NAMES  = ["Peak", "RMS"];

// ════════════════════════════════════════════════════════════════
// PRESET DATA — [threshold, ratio, attack, release, makeup, mix]
// ════════════════════════════════════════════════════════════════
const var presetData = [
    [-12,   4,   4,   300,  0, 100],
    [-18,   2,  10,   200,  3, 100],
    [-20, 3.5,   2,   100,  6, 100],
    [-15,   6, 0.5,    50,  4, 100],
    [-10,   4,   5,   200,  2, 100],
    [-30,   8,   1,    80,  0,  50],
    [-24,   2,  15,   400,  2, 100],
    [-16, 2.5,   8,   250,  1, 100],
    [-35,  20, 0.3,    30, 12, 100],
    [-22,   5,   3,   150,  5, 100]
];

// ════════════════════════════════════════════════════════════════
// A/B  ·  UNDO / REDO  ·  PRESET  ·  SWEET SPOT
// ════════════════════════════════════════════════════════════════

inline function saveCurrentToSlot(slot)
{
    slot[0]  = knbThreshold.getValue();
    slot[1]  = knbRatio.getValue();
    slot[2]  = knbAttack.getValue();
    slot[3]  = knbRelease.getValue();
    slot[4]  = knbVibeWheel.getValue();
    slot[5]  = knbCloudGain.getValue();
    slot[6]  = knbMix.getValue();
    slot[7]  = knbMakeup.getValue();
    slot[8]  = knbLookahead.getValue();
    slot[9]  = knbStereoLink.getValue();
    slot[10] = knbScHpf.getValue();
    slot[11] = knbScLpf.getValue();
    slot[12] = cmbKnee.getValue();
    slot[13] = cmbImpedance.getValue();
    slot[14] = cmbOversampling.getValue();
    slot[15] = btnDualStack.getValue();
    slot[16] = btnMsMode.getValue();
    slot[17] = btnAutoRelease.getValue();
    slot[18] = btnAutoMakeup.getValue();
};

inline function loadSlot(slot)
{
    isLoadingSlot = 1;
    knbThreshold.setValue(slot[0]);   knbThreshold.changed();
    knbRatio.setValue(slot[1]);       knbRatio.changed();
    knbAttack.setValue(slot[2]);      knbAttack.changed();
    knbRelease.setValue(slot[3]);     knbRelease.changed();
    knbVibeWheel.setValue(slot[4]);   knbVibeWheel.changed();
    knbCloudGain.setValue(slot[5]);   knbCloudGain.changed();
    knbMix.setValue(slot[6]);         knbMix.changed();
    knbMakeup.setValue(slot[7]);      knbMakeup.changed();
    knbLookahead.setValue(slot[8]);   knbLookahead.changed();
    knbStereoLink.setValue(slot[9]);  knbStereoLink.changed();
    knbScHpf.setValue(slot[10]);      knbScHpf.changed();
    knbScLpf.setValue(slot[11]);      knbScLpf.changed();
    cmbKnee.setValue(slot[12]);       cmbKnee.changed();
    cmbImpedance.setValue(slot[13]);  cmbImpedance.changed();
    cmbOversampling.setValue(slot[14]); cmbOversampling.changed();
    btnDualStack.setValue(slot[15]);  btnDualStack.changed();
    btnMsMode.setValue(slot[16]);     btnMsMode.changed();
    btnAutoRelease.setValue(slot[17]); btnAutoRelease.changed();
    btnAutoMakeup.setValue(slot[18]); btnAutoMakeup.changed();
    isLoadingSlot = 0;
};

inline function captureUndo()
{
    saveCurrentToSlot(undoState);
};

inline function loadPreset(idx)
{
    if (idx < 1) return;
    if (idx > presetData.length) return;

    isLoadingSlot = 1;
    local p = presetData[idx - 1];

    knbThreshold.setValue(p[0]);  knbThreshold.changed();
    knbRatio.setValue(p[1]);      knbRatio.changed();
    knbAttack.setValue(p[2]);     knbAttack.changed();
    knbRelease.setValue(p[3]);    knbRelease.changed();
    knbMakeup.setValue(p[4]);     knbMakeup.changed();
    knbMix.setValue(p[5]);        knbMix.changed();
    isLoadingSlot = 0;
};

inline function applySweetSpot()
{
    isLoadingSlot = 1;
    knbThreshold.setValue(-18);   knbThreshold.changed();
    knbRatio.setValue(3.5);       knbRatio.changed();
    knbAttack.setValue(5);        knbAttack.changed();
    knbRelease.setValue(120);     knbRelease.changed();
    knbMakeup.setValue(4);        knbMakeup.changed();
    knbMix.setValue(100);         knbMix.changed();
    knbVibeWheel.setValue(15);    knbVibeWheel.changed();
    isLoadingSlot = 0;
};

// Capture initial defaults into both A/B slots
saveCurrentToSlot(abSlotA);
saveCurrentToSlot(abSlotB);
saveCurrentToSlot(undoState);

// ════════════════════════════════════════════════════════════════
// TIMER — repaint meters at ~30 fps
// ════════════════════════════════════════════════════════════════
const var pnlTimer = Content.addPanel("pnlTimer", 0, 0);
pnlTimer.set("width",  0);
pnlTimer.set("height", 0);
pnlTimer.set("visible", false);
pnlTimer.startTimer(33);

pnlTimer.setTimerCallback(function()
{
    // Smooth-decay all meter values toward zero
    grValue   = grValue   * 0.85;
    inLevelL  = inLevelL  * 0.90;
    inLevelR  = inLevelR  * 0.90;
    outLevelL = outLevelL * 0.90;
    outLevelR = outLevelR * 0.90;

    // Peak-hold with slow decay
    if (Math.abs(grValue) > Math.abs(grPeak))
        grPeak = grValue;
    else
        grPeak = grPeak * 0.97;

    grReadout = grPeak;

    pnlInputMeter.repaint();
    pnlGrMeter.repaint();
    pnlOutputMeter.repaint();
    pnlGrReadout.repaint();
});

// ════════════════════════════════════════════════════════════════
// STANDARD CALLBACKS
// ════════════════════════════════════════════════════════════════
function onNoteOn()      {}
function onNoteOff()     {}
function onController()  {}
function onTimer()       {}

function onControl(number, value)
{
    // Capture undo state before any parameter change (skip during batch loads)
    if (!isLoadingSlot &&
        number != btnUndo && number != btnRedo && number != btnAB &&
        number != cmbPreset && number != btnAutoSweetSpot)
    {
        captureUndo();
    }

    // ── Dynamics processor wiring ──
    if (number == knbThreshold)
    {
        Compressor.setAttribute(Compressor.CompressorThreshold, value);
        tooltipText = "Threshold: " + value + " dB";
        pnlCurve.repaint();
    }

    if (number == knbRatio)
    {
        Compressor.setAttribute(Compressor.CompressorRatio, value);
        tooltipText = "Ratio: " + value + " :1";
        pnlCurve.repaint();
    }

    if (number == knbAttack)
    {
        Compressor.setAttribute(Compressor.CompressorAttack, value);
        tooltipText = "Attack: " + value + " ms";
    }

    if (number == knbRelease)
    {
        Compressor.setAttribute(Compressor.CompressorRelease, value);
        tooltipText = "Release: " + value + " ms";
    }

    if (number == knbMakeup)
    {
        Compressor.setAttribute(Compressor.CompressorMakeup, value);
        tooltipText = "Makeup: " + value + " dB";
    }

    // ── Controls stored for future C++ DSP ──
    if (number == knbVibeWheel)
        tooltipText = "Vibe: " + value + " %";

    if (number == knbCloudGain)
        tooltipText = "Cloud Gain: " + value + " dB";

    if (number == knbMix)
        tooltipText = "Mix: " + value + " %";

    if (number == knbLookahead)
        tooltipText = "Lookahead: " + value + " ms";

    if (number == knbStereoLink)
        tooltipText = "Stereo Link: " + value + " %";

    if (number == knbScHpf)
        tooltipText = "SC HPF: " + value + " Hz";

    if (number == knbScLpf)
        tooltipText = "SC LPF: " + value + " Hz";

    if (number == cmbKnee && value > 0 && value <= KNEE_NAMES.length)
        tooltipText = "Knee: " + KNEE_NAMES[value - 1];

    if (number == cmbImpedance && value > 0 && value <= IMP_NAMES.length)
        tooltipText = "Impedance: " + IMP_NAMES[value - 1];

    if (number == cmbOversampling && value > 0 && value <= OS_NAMES.length)
        tooltipText = "Oversampling: " + OS_NAMES[value - 1];

    if (number == cmbDetector && value > 0 && value <= DET_NAMES.length)
        tooltipText = "Detector: " + DET_NAMES[value - 1];

    if (number == btnDualStack)
        tooltipText = value ? "Dual Stack: ON" : "Dual Stack: OFF";

    if (number == btnMsMode)
        tooltipText = value ? "Mid/Side: ON" : "Mid/Side: OFF";

    if (number == btnAutoRelease)
        tooltipText = value ? "Auto Release: ON" : "Auto Release: OFF";

    if (number == btnAutoMakeup)
        tooltipText = value ? "Auto Makeup: ON" : "Auto Makeup: OFF";

    if (number == btnEcoMode)
        tooltipText = value ? "Eco Mode: ON" : "Eco Mode: OFF";

    // ── A/B toggle ──
    if (number == btnAB)
    {
        if (abState == 0)
        {
            saveCurrentToSlot(abSlotA);
            loadSlot(abSlotB);
            abState = 1;
            tooltipText = "Switched to slot B";
        }
        else
        {
            saveCurrentToSlot(abSlotB);
            loadSlot(abSlotA);
            abState = 0;
            tooltipText = "Switched to slot A";
        }
    }

    // ── Undo ──
    if (number == btnUndo)
    {
        if (value)
        {
            saveCurrentToSlot(redoState);
            loadSlot(undoState);
            tooltipText = "Undo";
        }
    }

    // ── Redo ──
    if (number == btnRedo)
    {
        if (value)
        {
            saveCurrentToSlot(undoState);
            loadSlot(redoState);
            tooltipText = "Redo";
        }
    }

    // ── Preset ──
    if (number == cmbPreset)
    {
        captureUndo();
        loadPreset(value);
        tooltipText = "Preset loaded";
    }

    // ── Sweet Spot ──
    if (number == btnAutoSweetSpot)
    {
        if (value)
        {
            captureUndo();
            applySweetSpot();
            tooltipText = "Sweet Spot applied";
        }
    }

    // Refresh tooltip display
    pnlTooltip.repaint();
}
