import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import ArchitectureDiagram from '../ArchitectureDiagram';
import { Link} from "react-router-dom";
import useMediaQuery from '@mui/material/useMediaQuery';
import { FigureCaption,SmallPrint, P, Ul,Subheading } from './documentation/StandardFormats';
import { Lv2cCopyrightText, Lv2cVersion, Lv2cVersionNotice } from '../VersionInfo';
import { Lv2Cairo } from './documentation/StandardFormats';



const About = ()=> {
    const floatLogo = useMediaQuery('(min-width:600px)');

    return (
        <Box className={"page_frame"}>
            <div className={"page_column"}>
                <img src="img/LvcLogo.svg" 
                    style={{position: "absolute", left: -90,top: -2, height: 76, 
                        display: floatLogo? undefined: "none"}} />

                <Typography variant="h2" sx={{ marginBottom: 4 }}>
                    <span style={{fontWeight: 900, color: "#505050"}}>Sponsorship</span>
                </Typography>
                <P>
                    <Lv2Cairo/> accepts sponsorship donations through Github Sponsors. Click <a href='https://github.com/sponsors/rerdavies'>here</a> to make a donation.
                </P>
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
export default About;
