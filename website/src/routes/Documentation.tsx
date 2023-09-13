import Box from '@mui/material/Box';
import DocFooter from './documentation/DocFooter';
import loadable from '@loadable/component';
import pMinDelay from 'p-min-delay';


const DocIndex = loadable(()=>pMinDelay(import('./documentation/DocIndex'),200));
const WhyLv2Lvtk =loadable(()=>pMinDelay(import('./documentation/WhyLVTK'),200));
const DocOverview =loadable(()=>pMinDelay(import('./documentation/DocOverview'),200));
const CodingConventions =loadable(()=>pMinDelay(import('./documentation/DocCodingStyle'),200));
const DocNoMatch =loadable(()=>pMinDelay(import('./documentation/DocNoMatch'),200));
const DocGettingStarted =loadable(()=>pMinDelay(import('./documentation/DocGettingStarted'),200));
const DocAPI =loadable(()=>pMinDelay(import('./documentation/DocAPI'),200));

import { Routes, Route  } from "react-router-dom";
const BuildingAnLv2UiPlugin =loadable(()=>pMinDelay(import('././documentation/BuildingAnLv2UiPlugin'),200));
const BuildingAnLv2AudioPlugin =loadable(()=>pMinDelay(import('././documentation/BuildingAnLv2AudioPlugin'),200));


function Documentation()
{
    return (
        <Box sx={{position: "absolute", 
                left: 0, top: 0, right: 0, bottom:0,
                display: "flex", flexFlow: "column", flexWrap: "nowrap"}}>
            <Box sx={{flex: "1 1 auto", position: "relative" }}>
                <Box className={"page_frame"} >
                    <Routes>
                        <Route path="index" element={<DocIndex/>} />
                        <Route path="why" element={<WhyLv2Lvtk/>} />
                        <Route path="getting_started" element={<DocGettingStarted/>} />
                        <Route path="building_an_lv2_audio_plugin" element={<BuildingAnLv2AudioPlugin/>} />
                        <Route path="building_an_lv2_ui_plugin" element={<BuildingAnLv2UiPlugin/>} />
                        <Route path="overview" element={<DocOverview/>} />
                        <Route path="coding_conventions" element={<CodingConventions/>} />
                        <Route path="apis" element={<DocAPI/>} />
                        <Route path="*" element={<DocNoMatch />} /> 
                    </Routes>


                </Box>
            </Box>
            {/* Footer */}
            <DocFooter sx={{
                flex: "0 0 auto"
                
            }} />
        </Box>
    );

}
export default Documentation;