import { useLocation } from "react-router-dom";
import { Link } from "react-router-dom";


export class NavPage {
    constructor(
        route: string, name: string, module: string,children: NavPage[]) {
        this.route = route; this.name = name; this.children = children; this.module = module;
    }
    route: string = "";
    name: string = "";
    module: string="";
    children: NavPage[] = []

    link = () => {
        return (<Link to={this.route}>
            {this.name}
        </Link>);
    }
    // Route = () => {
    //     return (
    //         <Route key={this.route} path={this.route} element={this.lazyLoader} />
    //     );
    // }
}

export interface NavInfo {
    previous?: NavPage;
    next?: NavPage;
    up?: NavPage;
    current: NavPage;
}

const docIndex: NavPage =
    new NavPage
        (
            "/documentation/index",
            "Index",
            "DocIndex",
            [
                new NavPage(
                    "/documentation/why",
                    "Why LVtk?",
                    "WhyLv2Lvtk",
                    []
                ),
                new NavPage(
                    "/documentation/overview",
                    "Overview",
                    "DocOverview",
                    []
                ),
                new NavPage(
                    "/documentation/getting_started",
                    "Getting Started",
                    "DocNoMatch",
                    []
                ),
                new NavPage(
                    "/documentation/building_an_lv2_audio_plugin",
                    "Building an LV2 Audio Plugin",
                    "DocNoMatch",
                    []
                ),
                new NavPage(
                    "/documentation/building_an_lv2_ui_plugin",
                    "Building an LV2 UI Plugin",
                    "BuildingAnLv2UiPlugin",
                    []
                ),
                new NavPage(
                    "/documentation/apis",
                    "API Documentation",
                    "DocAPI",
                    []
                ),
                new NavPage(
                    "/documentation/coding_conventions",
                    "LVtk Coding Conventions",
                    "DocNoMatch",
                    []
                )
            ]
        );

export function GetDocIndex(): NavPage {
    return docIndex;
}

// function getRoutes(navPage: NavPage,routes: JSX.Element[])
// {
//     routes.push(navPage.Route());
//     for (let i = 0; i < navPage.children.length; ++i)
//     {
//         getRoutes(navPage.children[i],routes);
//     }

// }
// export function GetRoutes()
// {
//     let routes: JSX.Element[] = [];
//     getRoutes(docIndex,routes);
//     return routes;
// }
function buildNav(location: string, navPage: NavPage) {
    if (navPage.route === location) {
        return {
            next: undefined,
            previous: undefined,
            up: undefined,
            current: navPage
        };
    }
    for (let i = 0; i < navPage.children.length; ++i) {
        const child = navPage.children[i];
        if (child.route == location) {
            return {
                previous: i > 0 ? navPage.children[i - 1] : undefined,
                next: i < navPage.children.length - 1 ? navPage.children[i + 1] : undefined,
                up: navPage,
                current: navPage.children[i]
            };
        } else if (location.startsWith(child.route)) {
            return buildNav(location, child);
        }
    }
    return undefined;

}

function DocNavIndex(): NavInfo | undefined {
    const location = useLocation().pathname;

    return buildNav(location, docIndex);
}

function FindNavPage(navInfo: NavPage, module: string): NavPage | null {
    if (navInfo.module === module) {
        return navInfo;
    }
    for (let i = 0; i < navInfo.children.length; ++i) {
        let t = FindNavPage(navInfo.children[i], module);
        if (t) {
            return t;
        }
    }
    return null;

}
export function PageNavLink(navPage: NavPage) {
    return (<Link to={navPage.module}>
        {navPage.name}
    </Link>);

}
export function DocNavLink(props: { module: string }) {
    const navPage = FindNavPage(docIndex, props.module);
    if (navPage) {
        return PageNavLink(navPage);
    }
    return (<div>
        Error: module not found: {props.module}
    </div>)
}

export default DocNavIndex;