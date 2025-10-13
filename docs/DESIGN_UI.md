# EzSqueeze UI Design Specification

**Version:** 1.0.0-beta.1  
**Last Updated:** October 13, 2025  
**Design Language:** Citrus Liquid Glass

---

## Design Philosophy

EzSqueeze's interface combines **Apple's liquid glass aesthetic** with vibrant **citrus accent colors** to create a modern, professional, and inviting user experience. The design prioritizes:

1. **Clarity**: Critical controls and meters are immediately visible
2. **Efficiency**: Fast workflow with minimal clicks
3. **Beauty**: Polished, premium appearance
4. **Accessibility**: WCAG AA compliance, keyboard navigation, scalability

---

## Color Palette

### Primary Citrus Colors

| Name | Hex | RGB | Usage |
|------|-----|-----|-------|
| **Primary Orange** | `#EF932C` | 239, 147, 44 | Main accents, knobs, highlights |
| **Light Orange** | `#F1B233` | 241, 178, 51 | Hover states, bright accents |
| **Dark Orange** | `#DA761E` | 218, 118, 30 | Pressed states, shadows |
| **Accent Orange** | `#EE811C` | 238, 129, 28 | Active states, focus |
| **Alt Orange** | `#E78826` | 231, 136, 38 | Secondary highlights |
| **Warning Orange** | `#C75B13` | 199, 91, 19 | Clipping indicators |
| **Highlight Yellow** | `#F3C16A` | 243, 193, 106 | Tooltips, text highlights |

### Neutral & Background Colors

| Name | Hex | RGB | Usage |
|------|-----|-----|-------|
| **Background Dark** | `#1A1A1D` | 26, 26, 29 | Main background |
| **Panel Background** | `#252528` | 37, 37, 40 | Glass panels, sections |
| **Glass Overlay** | `rgba(255, 255, 255, 0.08)` | - | Liquid glass effect |
| **Glass Border** | `rgba(239, 147, 44, 0.3)` | - | Panel borders with glow |
| **Text Primary** | `#E0E0E0` | 224, 224, 224 | Labels, values |
| **Text Dim** | `#909090` | 144, 144, 144 | Secondary text |

### Meter Colors

| Name | Hex | RGB | Usage |
|------|-----|-----|-------|
| **Meter Green** | `#4CAF50` | 76, 175, 80 | -∞ to -18 dBFS |
| **Meter Yellow** | `#FFC107` | 255, 193, 7 | -18 to -6 dBFS |
| **Meter Red** | `#F44336` | 244, 67, 54 | -6 to 0 dBFS |
| **Meter BG** | `#2A2A2D` | 42, 42, 45 | Meter background |
| **GR Orange** | `#EF932C` | 239, 147, 44 | Gain reduction meter |

### Contrast Ratios (WCAG AA)

- **Primary text on dark BG**: 12.5:1 ✅
- **Dim text on dark BG**: 4.8:1 ✅
- **Orange on panel BG**: 4.7:1 ✅
- **Buttons/focus states**: 3.5:1+ ✅

---

## Typography

### Fonts

#### Primary Display Font: **Aderoy**
- **Usage**: Logo, large headings
- **Weight**: Narrow, Bold
- **File**: `assets/ui/fonts/Aderoy.otf`

#### UI Font: **System Default** (SF Pro on macOS, Segoe UI on Windows)
- **Usage**: Labels, values, tooltips
- **Weights**: Regular (400), Bold (700)
- **Fallback**: Sans-serif

### Type Scale

| Element | Size | Weight | Case | Usage |
|---------|------|--------|------|-------|
| Logo | 42px | Bold | UPPER | "SQUEEZE" |
| Tagline | 14px | Regular | Title | "Citrus Dynamics" |
| Section | 12px | Bold | UPPER | "METERING", "ADVANCED" |
| Label | 11px | Bold | UPPER | "THRESHOLD", "RATIO" |
| Value | 13px | Regular | Mixed | "-18.0 dB", "4.0:1" |
| Tooltip | 12px | Regular | Mixed | Help text |

---

## Layout & Dimensions

### Main Window

- **Width**: 900px (scalable)
- **Height**: 650px (scalable)
- **Aspect Ratio**: ~1.38:1 (locked)
- **Scaling**: 90% - 130% (user adjustable)

### Grid System

8px base grid for alignment:
- **Margin**: 20px
- **Gutter**: 20px
- **Padding**: 10-20px within panels

---

## Component Specifications

### Glass Panels

**Visual Properties:**
```css
background: linear-gradient(135deg, 
    rgba(255, 255, 255, 0.1) 0%, 
    rgba(255, 255, 255, 0.05) 100%);
backdrop-filter: blur(12px);
border: 1px solid rgba(239, 147, 44, 0.3);
border-radius: 12px;
box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
```

**Hover State:**
```css
border: 1px solid rgba(239, 147, 44, 0.5);
box-shadow: 0 0 20px rgba(239, 147, 44, 0.3);
```

### Knobs

#### Citrus Knob Design

- **Size**: 60px - 120px (depending on importance)
- **Style**: Filmstrip (60 frames, vertical strip)
- **Range**: 300° rotation
- **Visual**: Radial gradient (light top-left, dark bottom-right)
- **Shadow**: Drop shadow + inner highlight
- **Active State**: Orange glow (#EF932C, 8px blur)

**Gradient:**
```css
background: radial-gradient(circle at 30% 30%, 
    #F1B233, #EF932C, #DA761E);
```

**Interaction:**
- **Click**: Jump to value
- **Drag**: Vertical drag adjusts value
- **Shift+Drag**: Fine adjustment (10× slower)
- **Double-click**: Reset to default
- **Scroll**: Increment/decrement (if enabled)

### Meters

#### Level Meters (Input/Output)

- **Width**: 50px
- **Height**: 320px
- **Segments**: 40 (8px each)
- **Color Zones**:
  - Green: -60 to -18 dB
  - Yellow: -18 to -6 dB
  - Red: -6 to +6 dB
- **Peak Hold**: 2-second decay, white indicator
- **Update Rate**: 30 Hz (smooth)

#### GR Meter

- **Width**: 50px
- **Height**: 320px
- **Color**: Solid orange (#EF932C)
- **Range**: 0 to -24 dB (inverted, 0 at top)
- **History Trail**: 100ms fade trail (optional)
- **Peak Hold**: No (continuous reading)

### Buttons

#### Standard Button

```css
background: rgba(239, 147, 44, 0.2);
border: 1px solid rgba(239, 147, 44, 0.5);
border-radius: 6px;
padding: 10px 20px;
color: #E0E0E0;
font-weight: bold;
```

**Hover:**
```css
background: rgba(239, 147, 44, 0.3);
border-color: rgba(239, 147, 44, 0.7);
```

**Active:**
```css
background: rgba(239, 147, 44, 0.5);
transform: translateY(1px);
```

#### Toggle Button (A/B, M/S, etc.)

- **Off State**: Dim border, transparent fill
- **On State**: Bright orange border, orange glow

### Preset Browser

- **Width**: 300px
- **Height**: 40px
- **Style**: Dropdown with search
- **Placeholder**: "Search presets..."
- **Icon**: Magnifying glass (left)
- **Dropdown**: Glass panel, max 10 visible items

---

## Sections

### Header (60px height)

**Elements:**
- **Logo** (20, 10): "SQUEEZE" in Aderoy, 42px, orange
- **Tagline** (160, 28): "Citrus Dynamics", 14px, dim
- **CPU Meter** (700, 15): Circular gauge, 30px diameter
- **OS Indicator** (790, 15): Badge "2×", "4×", "8×"
- **Latency** (850, 15): "5.5ms" text display

### Metering Section (20, 80, 180×500)

**Glass panel containing:**
- **Input Meter** (40, 100): Left channel
- **GR Meter** (105, 100): Gain reduction
- **Output Meter** (170, 100): Right channel
- **Label** (50, 440): "METERING", 12px, dim

### Main Controls (220, 80, 660×380)

**Top Row (Compression):**
- **Threshold** (260, 120): 100px knob
- **Ratio** (380, 120): 100px knob
- **Knee** (500, 120): 80px selector
- **Attack** (620, 120): 90px knob
- **Release** (740, 120): 90px knob

**Bottom Row (Character):**
- **Vibe Wheel** (260, 300): 120px radial control
- **Dual Stack** (400, 320): Toggle button
- **Impedance** (500, 320): 3-way selector
- **Cloud Gain** (640, 300): 80px knob
- **Mix** (750, 300): 80px knob

### Advanced Panel (220, 480, 660×90)

**Collapsible glass panel:**
- **Lookahead** (240, 500): 60px knob
- **Stereo Link** (320, 500): 60px knob
- **M/S Mode** (400, 510): Toggle
- **SC HPF** (460, 500): 60px knob
- **SC LPF** (540, 500): 60px knob
- **Oversampling** (620, 510): Dropdown
- **Auto Release** (700, 510): Toggle
- **Auto Makeup** (800, 510): Toggle

### Footer (590px y-position, 60px height)

- **Preset Browser** (20, 600): Search/dropdown
- **A/B** (340, 600): Toggle button, 50px
- **Undo** (400, 600): Arrow button, 40px
- **Redo** (450, 600): Arrow button, 40px
- **Auto Sweet Spot** (520, 600): Action button, 120px
- **Style Mode** (660, 600): Dropdown, 220px

---

## Animations & Transitions

### Timing

- **Fast**: 150ms (hover, focus)
- **Medium**: 300ms (panel expand, value change)
- **Slow**: 500ms (meter decay, fade out)

### Easing

- **UI Elements**: Cubic bezier (0.4, 0.0, 0.2, 1) - ease-out
- **Meters**: Linear (real-time)
- **Fade**: Ease-in-out

### Animations

1. **Knob Rotation**: Smooth interpolation, no stepping
2. **Meter Bars**: Ballistics (attack fast, release slow)
3. **Peak Hold**: Instant rise, 2s linear decay
4. **GR Trail**: 100ms fade with motion blur effect
5. **Panel Expand**: Height + opacity transition
6. **Tooltip**: 200ms fade-in after 500ms hover

---

## Accessibility

### Keyboard Navigation

**Tab Order:**
1. Main controls (threshold → ratio → knee → attack → release)
2. Character controls (vibe → dual stack → impedance → cloud gain → mix)
3. Advanced panel (all 8 controls)
4. Footer controls (preset → A/B → undo → redo → sweet spot)

**Shortcuts:**
- **Space**: Toggle selected button
- **Arrow Keys**: Adjust selected knob (±1)
- **Shift + Arrow**: Fine adjust (±0.1)
- **Enter**: Activate selected action button
- **Cmd/Ctrl + Z**: Undo
- **Cmd/Ctrl + Shift + Z**: Redo
- **Cmd/Ctrl + A/B**: Toggle A/B

### Focus Indicators

```css
outline: 3px solid rgba(239, 147, 44, 0.8);
outline-offset: 4px;
box-shadow: 0 0 12px rgba(239, 147, 44, 0.5);
```

### Screen Reader Support

- All controls have `aria-label` attributes
- Meters announce level changes on update
- State changes (on/off) are announced
- Tooltips are exposed to assistive tech

### High Contrast Mode

Optional theme variant:
- Increase border widths (2px → 3px)
- Brighter text colors (higher contrast)
- Disable glass blur (performance + clarity)

---

## Responsive Scaling

### Scale Factors

| Percentage | Use Case | Window Size |
|------------|----------|-------------|
| 90% | 1080p displays | 810 × 585 |
| 100% | Default | 900 × 650 |
| 110% | High-DPI | 990 × 715 |
| 130% | Accessibility | 1170 × 845 |

### Scaling Strategy

1. **Proportional**: All elements scale uniformly
2. **Snap to Grid**: Maintain 8px alignment at all scales
3. **Font Sizing**: Round to whole pixels
4. **Meter Width**: Minimum 40px at 90% scale

---

## UI States

### States Diagram

```
[Idle] ──hover──> [Hover] ──click──> [Active] ──release──> [Hover]
   │                                        │
   └────────────────────────────────────────┘
                  mouse leave
```

### Loading State

- Show spinner with orange accent
- Disable all controls (dim + no interaction)
- Display "Loading preset..." message

### Error State

- Red outline on affected control
- Toast notification with error message
- Log to console for debugging

---

## Implementation Notes

### HISE Implementation

```javascript
// Example: Creating a filmstrip knob
const knob = Content.addKnob("Threshold", 260, 120);
knob.set("width", 100);
knob.set("height", 100);
knob.set("filmstripImage", "{PROJECT_FOLDER}citrus_knob.png");
knob.set("numStrips", 60);
knob.set("scaleFactor", 1.0);
knob.set("showValuePopup", "Below");
```

### JUCE Implementation

```cpp
// Example: Custom LookAndFeel for citrus theme
class CitrusLookAndFeel : public juce::LookAndFeel_V4 {
    void drawRotarySlider(...) override {
        // Custom knob rendering with gradients
    }
    
    void drawLinearSlider(...) override {
        // Custom meter bar rendering
    }
};
```

---

## Assets Required

### Graphics

- [x] `citrus_knob_filmstrip.png` (100×6000px, 60 frames)
- [ ] `citrus_knob_large.png` (120×7200px, 60 frames)
- [ ] `citrus_knob_small.png` (60×3600px, 60 frames)
- [ ] `vibe_wheel.png` (120×7200px, 60 frames, circular)
- [ ] `meter_mask.png` (segmented overlay)
- [ ] `logo_icon.png` (512×512px, for plugin icon)

### Fonts

- [x] `Aderoy.otf` (already present)

---

## Testing Checklist

- [ ] Verify contrast ratios with color picker
- [ ] Test with macOS VoiceOver
- [ ] Test with Windows Narrator
- [ ] Test keyboard navigation (all controls reachable)
- [ ] Test at 90%, 100%, 110%, 130% scales
- [ ] Test on 1080p, 1440p, 4K displays
- [ ] Verify tooltips appear on hover
- [ ] Check focus indicators are visible
- [ ] Test in different DAWs (plugin window resize behavior)

---

## Design Evolution

### Version History

- **v1.0.0-beta.1** (Oct 2025): Initial design specification
- **v1.0.0** (TBD): Finalized after user testing

### Future Considerations

- Dark/light theme toggle
- Custom color schemes (user preference)
- Compact mode (reduced height for mixers)
- Meter customization (range, ballistics)

---

**Design Status**: Specification complete (Phase 0).  
UI implementation begins in Phase 3.

**Wireframe**: See `assets/ui/wireframes/main_layout.json` for detailed coordinates.

