import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import useMediaQuery from '@mui/material/useMediaQuery';


import { Link} from "react-router-dom";

import { GetDocIndex } from './DocNavIndex';


function DocIndex() {
    const docIndex = GetDocIndex();
    const itemSx = { marginLeft: 4,marginBottom: 0.75};
    const floatLogo = useMediaQuery('(min-width:750px)');

    return (
        <Box >
            <Box className={"page_column"}>
                <img src="img/Lv2C-Logo-whitebg.svg" style={{position: "absolute", left: -138,top: -14, width: 125, display: floatLogo? undefined: "none"}} />
                <Typography variant="h3" style={{fontWeight: 1200}}>LVtk</Typography>
                <Typography variant="h6" paragraph>The Documentation</Typography>
                {
                    docIndex.children.map((page) => {
                        return (
                            <Box key={page.route} sx={itemSx}>
                                <Link to={page.route}>
                                    <Typography>
                                        {page.name}
                                    </Typography>
                                </Link>
                            </Box>
                        );
                    })
                }
            </Box>
        </Box>

    );
}

export default DocIndex;
