
import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';

import { useLocation} from "react-router-dom";


function NoMatch()
{
    const location = useLocation();

    return (
        <Box className={"page_frame"}>
        <Box className={"page_column"}>
            <Typography variant="h3" paragraph>Error: Page not found. </Typography>
            <Typography paragraph>
                <span className={"code"}>#{location.pathname}</span>
                </Typography>

            <Typography variant="body2" paragraph sx={{paddingLeft: "24px", paddingRight: "24px", paddingTop: "32px", color: "#00000080"}}>
            “You step in the stream,<br/> but the stream moves on.”
            </Typography>

        </Box>
    </Box>

    );
}

export default NoMatch;