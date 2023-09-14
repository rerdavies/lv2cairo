import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import { useLocation} from "react-router-dom";


function DocNoMatch() {

    const location = useLocation();
    return (
        <Box className={"page_column"}>
            <Typography variant="h3">LVtk</Typography>
            <Typography variant="h6" paragraph>The Documentation</Typography>

            <Typography paragraph>Page not found. <span className={"code"}>{location.pathname}</span></Typography>

            <Typography variant="body2" paragraph sx={{paddingLeft: "24px", fontStyle: "italic", paddingRight: "24px", paddingTop: "32px", color: "#404040" }}>
            “You step in the stream,<br/> but the stream moves on.”
            </Typography>

        </Box>

    );
}


export default DocNoMatch;