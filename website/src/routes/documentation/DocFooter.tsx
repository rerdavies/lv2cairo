import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import ButtonBase from '@mui/material/ButtonBase';
import { SxProps } from '@mui/material/';
import DocNavIndex, { NavInfo, NavPage } from './DocNavIndex';

import NavigateNextIcon from '@mui/icons-material/NavigateNext';
import NavigateBeforeIcon from '@mui/icons-material/NavigateBefore';
import ArrowUpwardIcon from '@mui/icons-material/ArrowUpward';
import { ReactElement } from 'react';

import { useNavigate } from "react-router-dom"

function NavLink(navPage: NavPage | undefined, icon: ReactElement, iconAfter?: boolean) {
    const sectionStyle = {
        flex: "0 1 auto",
        display: "flex", 
        flexDirection: "row", 
        flexWrap: "nowrap",
        alignItems: "center",
        margin: "0px 4px ",
        padding: "8px 16px",
        borderRadius: 1.5,
        '&:hover': {
            backgroundColor: "#80808040",
          }        
    };

    const navigate = useNavigate();

    const textStyle = {
        flex: "1 1 auto", marginLeft: "4px", textOverflow: "ellipsis" ,lineHeight: 1, textAlign: "start"
    };

    if (!navPage) {
        return undefined;
    }


    if (iconAfter) {
        return (
            <ButtonBase sx={sectionStyle} onClick={()=> { navigate(navPage.route); }}>
                <Typography variant="caption" noWrap component="div"
                    sx={{flex: "1 1 auto", marginLeft: "4px", textOverflow: "ellipsis" ,lineHeight: 1,textAlign: "right"}} 
                >
                    {
                        navPage.name.toUpperCase()
                    }
                </Typography>
                {icon}
            </ButtonBase>
        );
    }
    return (
        <ButtonBase sx={sectionStyle} onClick={()=> { navigate(navPage.route); }}
        >
            {icon}
            <Typography variant="caption" noWrap component="div"
                sx={textStyle}
            >
                {
                    navPage.name.toUpperCase()
                }
            </Typography>
        </ButtonBase>
    );
}

function DocFooter(args: { sx: SxProps }) {
    const navIndex: NavInfo | undefined = DocNavIndex();
    if (!navIndex) {
        return undefined;
    }

    const iconSx = { width: 16, height: 16 };
    return (
        <Box className={"nav_bar"}
            sx={args.sx}
            style={{
                display: "flex", flexDirection: "row", paddingLeft: 16, paddingRight: 16,
                alignItems: "center", flexWrap: "nowrap", height: "48px"
            }}>
            {NavLink(navIndex.up, (<ArrowUpwardIcon sx={iconSx} />))}

            <Box style={{flex: "1 1 1px"}}></Box>
            {NavLink(navIndex.previous, (<NavigateBeforeIcon sx={iconSx} />))}

            {NavLink(navIndex.next, (<NavigateNextIcon sx={iconSx} />), true)}

        </Box>
    )
    return ("<div/>");
}

export default DocFooter;