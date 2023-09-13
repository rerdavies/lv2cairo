import * as React from 'react';

import { useNavigate } from "react-router-dom"

import AppBar from '@mui/material/AppBar';
import Toolbar from '@mui/material/Toolbar';
import Typography from '@mui/material/Typography';
import ButtonBase from '@mui/material/ButtonBase';



const buttonStyle = { flexGrow: 0, paddingLeft: 1, paddingRight: 1, height: "48px", '&:hover': { background: "#FFFFFF20" }, borderRadius: 2 };

const Navbar = () => {
    const navigate = useNavigate();

    return (
        <AppBar sx={{ flex: "0 0 auto", paddingRight: 1, background: "#302030" }} position="relative">
            <Toolbar>

                <ButtonBase  sx={buttonStyle}  >
                    <img width="40px" height="40px"  src="img/LvkLogoDark.svg" />

                    <Typography variant="h6" component="div" sx={{marginLeft: 1}}
                        onClick={() => navigate("/")}>

                        LVTK
                    </Typography>
                </ButtonBase>
                <ButtonBase sx={buttonStyle}>
                    <Typography variant="h6" component="div" sx={{ cursor: "default", flexGrow: 0, opacity: 0.75 }}
                        onClick={() => navigate("/documentation/index")}
                    >
                        Documentation
                    </Typography>
                </ButtonBase>
                {/* <ButtonBase sx={buttonStyle}>
                    <Typography variant="h6" component="div" sx={{ cursor: "default", flexGrow: 0, opacity: 0.75 }}
                        onClick={() => navigate("/tutorials")}
                    >
                        Tutorials
                    </Typography>
                </ButtonBase> */}
                <ButtonBase sx={buttonStyle}>
                    <Typography variant="h6" component="div" sx={{ cursor: "default", flexGrow: 0, opacity: 0.75 }}
                        onClick={() => navigate("/about")}
                    >
                        Sponsor
                    </Typography>
                </ButtonBase>
                <div style={{ flexGrow: 1 }} >
                </div>
            </Toolbar>
        </AppBar>
    )
}
export default Navbar;