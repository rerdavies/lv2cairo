import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import { P } from './StandardFormats';


function WhyLVTK() {

    return (
        <Box className={"page_column"}>
            <Typography variant="h4" paragraph>Why LVtk?</Typography>

            <Typography variant="h6" paragraph>Host vs. Plugin UI Frameworks</Typography>
            <P>
                One of the significant challenges of writing user interfaces for LV2 plugins is that
                plugins cannot use major UI frameworks, like GTK+, GTK/3 or QT. LV2 plugins are loaded by LV2 hosts
                that determine which UI libraries get loaded. Since major UI frameworks can't be loaded simultaneously,
                Plugins that try to load an incompatible UI library will not load. LVtk has been designed principally
                to address this problem.
            </P>
            <P>
                The solution to the problem is to use the Cairo/X11 graphics library as a rendering backend. Cairo/X11 can coexist
                with other UI libraries. The downside is that X11/Cairo provides graphics rendering only, and does not
                provide widgets or UI controls of any kind. LVtk is intended to bridge that gap. 
            </P>
            <P>
                The recent release of Dave Robillard's <i>SUIL</i> library mitigates this problem to an extent. The SUIL library 
                allows LV2 hosts to run LV2 plugin user interfaces out of process, which solves the UI framework dependency
                problem. As more hosts integrate SUIL, LVtk will become less useful.
            </P>

            <Typography variant="h6" paragraph>Unpleasant Choices for MIT-Licensed Projects</Typography>

            <P>
                Even with SUIL in place, there remains a more difficult problem: which UI framework should you use for developing 
                LV2 plugins?
            </P><P> 
                With the introduction of GTK 4.0, it has become profoundly obvious that GTK is no longer a viable platform.
                GTK is no longer a viable developmet platform. Failing to learn lessons from the GTK 3.0 fiasco, GTK 4.0 completely breaks
                the GTK 3.0 API again. Porting an app from GTK 3.0 to GTK 4.0 is almost worse than starting fom scratch.
            </P>
            <P> Qt is a fine choice, save for the fact that Qt license are a deliberate minefield of GPL, LGPL and proprietary licenses. And 
                the documentation for Qt seems to have been deliberately constructed to make it difficult to determine ahead of time whether
                Qt will adequately adress your needs without tripping over a GPL license in a dependent module. Given the prohibitive cost of 
                a  commercial license, this makes Qt an unattractive choice.
            </P>
            <P>And JUICE suffers from the same problem, but without an LGPL option: the choice of a prohibitively expensive commercial license, or 
                a GPL license that precludes use of an MIT license, or potential use of code in commercial products.</P>
            <P>
                The LVtk library is MIT licensed (with LGPL dependencies on Cairo, and PangoCairo). End of that problem.
            </P>
            <Typography variant="h6" paragraph>Lv2Plugin's Place in the Library</Typography>
            <P> It's also worth discussing the Lv2Plugin's place in the LVtk library. </P>
            <P>
                The Lv2PLugin class is a late addition to the library. It's borrowed from the <a href="https://github.com/rerdavies/ToobAmp">ToobAmp project</a>'s source.
                It was originally brought in to allow implementaton of a test plugin for LVtk. But in the interest of providing a complete solution
                for writing LV2 plugins, it graduated to become a public feature of the LVtk library.</P>
                <P> Be advised that the LVPlugin API is not stable at present.
                The current recommendation is to cut and paste the LvPLugin.hpp and Lv2Plugin.cpp source files directly in your projects if you choose to use
                Lv2Plugin.</P>
            <P> Lv2Plugin was written with a vague plan to maybe release it outside of the ToobAmp project if there was a good reaso to do so.
                And now there is a good reason to do so. However, LV2Plugin still needs further review and refactoring to make it a more perfect basis for developing 
                LV2 audio plugins. It's still a good choice. Really. 
                But there are improvments that I'd like to make in the future, which will likely break parts of the present API, mostly relating to the 
                current impelementation of loading and saving state, and handling of background tasks.</P>
            <P>Doing this properly would require a fair bit of work. The current source is covered by automated and manual testing in the 
                ToobAmp project. Re-implemenation and refactoring would require test coverage in the LVtk project. So a decision was made to 
                defer this refactoring work until after the initial release.
            </P>
            <P>Feel free to use these features in the meantime; but be aware 
                that there may be imminent breaking changes to the APIs.
            </P>
        </Box>
    );
}


export default WhyLVTK;