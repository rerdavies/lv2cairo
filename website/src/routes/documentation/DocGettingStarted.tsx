import { Page,Heading, Code, P, Subheading, Ul } from './StandardFormats';
import { PreformatedText } from '../../CodeFragment';


function DocGettingStarted() {

    return (
        <Page>
            <Heading>Getting Started</Heading>

            <P>TBD</P>
            <Ul>
                <li>Getting the source</li>                
                <li>Dependencies</li>                
                <li>Building</li>                
                <li>The Template Project</li>                
            </Ul>
        <Subheading>Getting the source</Subheading>
        <P>The source code for LVTK is available at <br/>
        <Code>
            <a href={"https://github.com/rerdavies/lvtk"}>
            https://github.com/rerdavies/lvtk
            </a>
            </Code>
        </P>
        
        <Subheading>Dependencies</Subheading>
        <P>LVTK requires the following packages to be installed.</P>

        {PreformatedText(`
sudo apt install libcairo2-dev
sudo apt install libpangocairo-1.0-0
sudo apt install cmake
sudo apt install ninja-build #optional
sudo apt install catch2
sudo apt install librsvg2-dev
sudo apt install lv2-dev
sudo apt install liblilv-dev
`)}

    <Subheading>Build Procedure</Subheading>
    <P>LVTK was built using Visual Studio Code. If you open the folder in Visual Studio code, with CPP and CMake extensions installed, 
    </P>
    <P>If you prefer not to use Visual Studio Code, you can use the following commands to build LVTK.</P>

    <P>To configure the build procedure:</P>
    {PreformatedText(`
    TBD...
    (Completely generic cmake build procedure)
`)}
    </Page>
)}



export default DocGettingStarted;