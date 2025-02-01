const pagefind = await import("../pagefind/pagefind.js");

pagefind.init();

const searchContainer = document.getElementById("nav-search");
const searchBox = searchContainer.firstElementChild;
searchBox.disabled = false; // If user has not enabled JS, the search box will be disabled

const resultsContainer = document.getElementById("search-results");
const resultsList = resultsContainer.firstElementChild;

const process = (results) => {
    if (searchBox.value === "") {
        resultsContainer.style.display = "none";
        return;
    }
    resultsContainer.style.display = "flex";

    if (results.length == 0) {
        resultsList.innerHTML = `<div class="search-count">No results found</div>`;
        return;
    }

    const stripHtml = (url) => url.replace(/\.html($|\?)/, "");

    resultsList.innerHTML = `<div class="search-count">Search results:</div>` +
        '<ul>' + results.map((result) => {
        const title = `<a href="${stripHtml(result.url)}"><span>${result.meta.title}</span></a>`;
        const subResults = (result.sub_results?.length ?? 0) > 0 ?
            `<ul>${result.sub_results.map(subResult => {
                const title = `<a href="${stripHtml(subResult.url)}"><span>${subResult.title}</span></a>`;
                const excerpt = `<div class="search-excerpt">…${subResult.excerpt}…</div>`;
                return `<li>${title}${excerpt}</li>`;
                }).join('')}</ul>` :
            "";
        const excerpt = subResults === "" ? `<div class="search-excerpt">…${result.excerpt}…</div>` : "";
        return `<li>${title}${excerpt}${subResults}</li>`;
    }).join('') + '</ul>';
};

document.getElementById("search-clear").addEventListener("click", () => {
    searchBox.value = "";
    process([]);
});

searchBox.addEventListener("input", async (event) => {
    const search = await pagefind.debouncedSearch(searchBox.value, {}, 200/*ms*/);
    if (search === null) {
        // a more recent search call has been made, nothing to do
        return;
    }
    const topResults = await Promise.all(search.results.slice(0, 5).map(r => r.data()));
    process(topResults);
});