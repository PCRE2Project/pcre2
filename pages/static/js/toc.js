(() => {
    const toc = document.querySelector('[role="doc-toc"]');
    if (!toc) return;

    const headingSelector = "h1, h2, h3, h4, h5, h6";
    const entries = [...toc.querySelectorAll('a[href^="#"]')].flatMap(link => {
        const id = link.getAttribute("href").slice(1);
        let target = document.getElementById(id);
        if (!target) {
            try {
                target = document.getElementById(decodeURIComponent(id));
            } catch (error) {
                if (!(error instanceof URIError)) throw error;
                console.warn("Invalid Contents link fragment:", link.getAttribute("href"));
                return [];
            }
        }
        const heading = target?.matches(headingSelector) ? target : target?.querySelector(headingSelector);
        if (!heading || !heading.closest(".prose")) {
            console.warn("Contents link has no content heading:", link.getAttribute("href"));
            return [];
        }
        return [{link, heading}];
    });
    if (!entries.length) return;

    let active;
    let scheduled = false;
    const update = () => {
        scheduled = false;
        let visible;
        let previous;
        for (const entry of entries) {
            const bounds = entry.heading.getBoundingClientRect();
            if (bounds.bottom <= 0) {
                previous = entry;
            } else if (bounds.top < window.innerHeight) {
                visible = entry;
                break;
            }
        }
        // Between headings, retain the last one passed at the top of the page.
        const next = visible || previous;
        if (next === active) return;
        active?.link.removeAttribute("aria-current");
        next?.link.setAttribute("aria-current", "location");
        active = next;
    };
    const schedule = () => {
        if (scheduled) return;
        scheduled = true;
        requestAnimationFrame(update);
    };

    window.addEventListener("scroll", schedule, {passive: true});
    window.addEventListener("resize", schedule);
    window.addEventListener("pageshow", schedule);
    window.addEventListener("load", schedule);
    document.fonts.ready.then(schedule);
    update();
})();
