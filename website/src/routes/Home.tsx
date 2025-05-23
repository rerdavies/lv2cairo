import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import ArchitectureDiagram from '../ArchitectureDiagram';
import { Link} from "react-router-dom";
import useMediaQuery from '@mui/material/useMediaQuery';
import { FigureCaption,SmallPrint, P, Ul,Subheading } from './documentation/StandardFormats';
import { Lv2cCopyrightText, Lv2cVersion, Lv2cVersionNotice } from '../VersionInfo';
import { Lv2Cairo } from './documentation/StandardFormats';


const Home = () => {
    const floatLogo = useMediaQuery('(min-width:600px)');

    return (
        <Box className={"page_frame"}>
            <div className={"page_column"}>
                <img src="img/LvcLogo.svg" 
                    style={{position: "absolute", left: -90,top: -2, height: 76, 
                        display: floatLogo? undefined: "none"}} />

                <Typography variant="h2" sx={{ marginBottom: 4 }}>
                    <span style={{fontWeight: 900, color: "#505050"}}>Lv2Cairo</span>
                </Typography>
                <SmallPrint>
                    { Lv2cVersion() }
                </SmallPrint>
                <SmallPrint>
                    { Lv2cVersionNotice() }
                </SmallPrint>
                <P>
                    <Lv2Cairo/> is a C++ class library for building LV2 plugins with custom user 
                    interfaces, using Cairo/X11 as a graphics backend.</P>
                    <table>
                    <tr><td>
                        <img src="img/SamplePlugin.png" style={{width: "80%", marginLeft: 24, marginTop: 4, marginRight: 24, marginBottom: 4 }} />
                    </td></tr>
                    <tr><td>
                        <FigureCaption>
                            Fig 1: User interface generated from a manifest.tll file.
                        </FigureCaption>
                    </td></tr>
                </table>
                <P>Features:</P>
                <Ul>
                    <li><Typography>Light-weight graphical UI rendered using X11/Cairo.</Typography></li>
                    <li><Typography>MIT Licensed.</Typography></li>
                    <li><Typography>A rich set of input and output controls for use in LV2 plugins</Typography></li>
                    <li><Typography>Support for LV2 path properties</Typography></li>
                    <li><Typography>Host-independent file selection dialog.</Typography></li>
                    <li><Typography>Automatic generation of UI from manifest.tll files.</Typography></li>
                    <li><Typography>A rich set of features for producing customized plugin user interfaces.</Typography></li>
                </Ul>

                <Subheading>Architecture</Subheading>
                <ArchitectureDiagram/>
                <Ul>
                    <li><Typography>The <i>Lv2c</i> Element library provide a set of controls and widgets with layout, styling and theming that render user interfaces on a Cairo/X11 surface.</Typography> </li>
                    <li><Typography>The <i>Lv2UI</i> class is a base-class for implementing LV2 UI plugins.
                    </Typography></li>
                    <li><Typography>The <i>Lv2Plugin</i> class is a base-class for implementing LV2 audio plugins.</Typography></li>
                    <li><Typography>Given a description of an LV2 control port, the <i>Lv2PortViewFactory</i> class provides a view that can be used 
                    to display and edit port values in the UI. It provides rendering of standard controls (dials, VU meters, dropdowns, switches, etc.) while 
                    providing a point at which custom control can be injected into the Lv2UI's user interface, or where standard controls can be customized.
                    </Typography> </li>

                </Ul>
                <P>
                    The audio plugin and UI plugin implementations are completely independent. If you have an existing
                    LV2 audio plugin (or prefer to use another library), <Lv2Cairo/> is still an entirely suitable choice for building a user interface for your plugin.
                </P>

                <Subheading>Customization</Subheading>
                <P>
                    <Lv2Cairo/> can automatically generate serviceable custom user interfaces directly
                    from  <span className={"code"}> manifest.tll</span> files. At the same time, Lv2Cairo makes it easy to customize your UI plugin interface as well.
                </P>
                <img src="img/Customization.png" style={{width: "60%",float: "right", marginLeft: 16, marginRight: 24}} />
                <Typography paragraph style={{textAlign: "right", color: "#606060",fontSize: "0.9em", paddingTop: 24, paddingRight: 16}}>
                    Customize with styles,<br/>themes, or new  SVG<br/> and PNG elements.

                </Typography>

                <div style={{clear:"both"}}>
                </div>


                <div style={{clear:"both", height: 1, background: "#666", marginTop: 16, marginBottom: 32}}>
                    </div>

                <Typography paragraph variant="h4">
                    Where to Go from Here
                </Typography>
                
                <Box sx={{ paddingLeft: 4, marginBottom: 1.5 }}>
                    <Link to="/documentation/index"><Typography >The Documentation</Typography></Link>
                    <Box sx={{ paddingLeft: 2, marginTop: 1 }}>
                        <Link to="/documentation/overview"><Typography >More Details</Typography></Link>
                        <Link to="/documentation/getting_started"> <Typography>Getting Started</Typography></Link>
                        <Link to="/documentation/building_an_lv2_audio_plugin"> <Typography>Building an LV2 Audio Plugin</Typography></Link>
                        <Link to="/documentation/building_an_lv2_ui_plugin"> <Typography>Building an LV2 UI Plugin</Typography></Link>
                        <Link to="/documentation/apis"> <Typography>API Documentation</Typography></Link>
                        <Link to="/documentation/coding_conventions"> <Typography>Lv2Cairo Coding Conventions</Typography></Link>
                    </Box>
                    <br/>
                    
                    <a href="http://github.com/rerdavies/lv2cairo"> <Typography>The GitHub Repository</Typography></a>
                </Box>
                <div style={{height: 24}} />
                <div style={{height: 1, background:"#606060",marginTop: 16, marginBottom: 16}} />
                <Typography paragraph style={{fontSize: "0.8em"}}>
                    Lv2Cairo is provided under the terms of 
                    an <Link to="/license">MIT license</Link>.
                    <br/>{Lv2cCopyrightText()}
                </Typography>



            </div>
        </Box>
    );

}
export default Home;
