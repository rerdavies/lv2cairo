import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import useMediaQuery from '@mui/material/useMediaQuery';


function ArchitectureDiagram() {
    const floatArchitecture = useMediaQuery('(min-width:1356px)');
    return (
        < Box sx={floatArchitecture ? {
            position: "absolute", left: "100%",
            marginLeft: 3,
            width: "410px"
        } : {
            display: "flex", flexFlow: "column",
            marginLeft: "auto",
            marginRight: "auto",
            marginTop: 1,
            marginBottom: "24px",
            width: "410px"

        }
        }>
            <Box style={{ border: "black 1px solid", background: "#FFFFFF" }}>
                <img src="img/architecture.svg" style={{width: 409.5, height: 348 }} />
            </Box>
            <Typography variant="caption" paragraph sx={{ textAlign: "center", marginTop: 1 }}>
                LVtk Architecture
            </Typography>
        </Box >
    );

}

export default ArchitectureDiagram;