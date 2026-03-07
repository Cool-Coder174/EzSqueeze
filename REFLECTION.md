 # Reflection Report — EzSqueeze Project

**Author:** Sara Zhao  
**Date:** March 6, 2026  
**Word count:** ~480

---

## Activity Description

**Expectations:** The EzSqueeze project aimed to deliver a professional dynamics compressor plugin for indie musicians and small producers. The expectations included a complete overhaul from an early prototype: a robust C++ DSP engine, a modern user interface with a distinct visual identity, comprehensive documentation, and a reliable build and release pipeline across multiple phases (Foundation, DSP, Advanced Features, UX, Presets, Testing, Build/CI, Documentation, Final Polish).

**What I did:** I focused on the **front end** and **documentation**. On the front end, I contributed to the user interface implementation—supporting the citrus liquid-glass theme, layout and controls (e.g. metering, preset browser, A/B comparison, tooltips), and ensuring the 900×650 professional UI met the design specification. I also helped with **documentation**: writing or updating project docs (e.g. README, design and build guides, user-facing or contributor-facing material) so that the codebase and workflow were clear for both developers and users.

---

## Technical Decisions

From a technical perspective, several decisions shaped my work:

- **Design system:** We committed to the Citrus Liquid Glass theme (colour palette, contrast, typography) and applied it consistently across the UI so the plugin felt cohesive and professional.
- **Accessibility and usability:** We prioritised WCAG AA contrast, keyboard navigation, and scalable UI (90–130%) so the plugin is usable for a wide range of users and setups.
- **Documentation structure:** We organised docs into clear categories (architecture, build, design, technical notes, user manual) and kept the README as the main entry point, with links to deeper guides.
- **Front-end–backend boundary:** I aligned my UI work with the existing HISE/JUCE and parameter layout so that controls, presets, and metering correctly reflected the DSP behaviour without introducing inconsistent behaviour.

These choices balanced aesthetics, accessibility, maintainability, and alignment with the rest of the team’s implementation.

---

## Contributions

I participated **as part of a team**. The project involved DSP implementation, build system, testing, and release preparation by other members (e.g. Isaac Hernandez and others noted in the repo).

**My specific contributions:**

- **Front end:** Implementing and refining the plugin UI—panels, controls, metering, preset browser, A/B snapshot UI, and tooltips—in line with the DESIGN_UI specification and the citrus theme.
- **Documentation:** Writing or revising markdown documentation (e.g. README, design guide, build instructions, and other docs in the `docs/` folder) to support both developers and end users.

I coordinated with the team on design tokens, parameter naming, and doc structure so the front end and documentation stayed consistent with the overall product vision.

---

## Quality Assessment

I would assess my participation as **solid but with room to improve**. I delivered on my assigned front-end and documentation tasks and helped the project reach a state where the UI and docs are in good shape for the current phase. The interface is coherent with the design spec, and the documentation is structured and findable.

**What I would do differently if I could redo the event:**

1. **Earlier design review:** I would ask for more frequent, short design/UX reviews with the team so layout and interaction details could be validated earlier and rework reduced.
2. **User testing:** I would push for at least one round of informal user testing (e.g. with a few musicians) to validate readability of meters, clarity of tooltips, and preset workflow before locking the UI.
3. **Documentation ownership:** I would define explicit ownership for each doc (e.g. who updates BUILD vs USER_MANUAL) and add a simple “last reviewed” or version note so we could track when docs were last checked against the code.
4. **Accessibility checks:** I would run a basic accessibility pass (e.g. contrast checker, tab order) from the start rather than only near the end, to catch issues when they are cheaper to fix.

Overall, the experience strengthened my skills in plugin UI implementation and technical documentation within a multi-phase, team-based project.
