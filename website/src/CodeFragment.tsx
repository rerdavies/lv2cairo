import {ReactNode,CSSProperties} from "react";
import Typography from "@mui/material/Typography";
import Box from "@mui/material/Box";
import { SxProps } from '@mui/material';



const CodeFragment = (props: {sx?: SxProps, style?: CSSProperties,children?: ReactNode}) => {
    return (
        <Box className={"code_fragment"} 
            sx={{
            whiteSpace: "pre", 
            border: "#C0C0C0 1pt solid", background: "#E0E0E0",
            padding: "8px 16px",
            margin: "0px 32px 16px 24px ",
            overflow: "auto",
             ...props.sx}}  >
            <Typography variant="inherit" children={props.children}  />
        </Box>
    )
}

export default CodeFragment;

export function PreformatedText(text: string)
{
    return (<CodeFragment> {text} </CodeFragment>);
}

