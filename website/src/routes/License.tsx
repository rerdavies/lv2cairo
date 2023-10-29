import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';


function License() {
    return (
        <Box className={"page_frame"}>
            <Box className={"page_column"}>
                <Typography variant="h3" sx={{ marginBottom: 4 }}>
                    Lv2Cairo License
                </Typography>
                <Typography paragraph sx={{ marginBottom: 5 }}>
                    The Lv2Cairo library is licensed under the following terms.
                </Typography>
                <Box style={{ marginLeft: "24px" }}>
                    <Box className={"license-text"}>
                        <Typography paragraph variant="inherit">
                            Copyright © 2023 Robin Davies
                        </Typography>
                        <Typography paragraph variant="inherit">
                            Permission is hereby granted, free of charge, to any person obtaining a copy of
                            this software and associated documentation files (the "Software"), to deal in
                            the Software without restriction, including without limitation the rights to
                            use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
                            the Software, and to permit persons to whom the Software is furnished to do so,
                            subject to the following conditions:
                        </Typography>
                        <Typography paragraph variant="inherit">

                            The above copyright notice and this permission notice shall be included in all
                            copies or substantial portions of the Software.
                        </Typography>
                        <Typography paragraph variant="inherit">

                            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
                            IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
                            FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
                            COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
                            IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
                            CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

                        </Typography>
                    </Box>
                </Box>
            </Box >
        </Box>
    );

}
export default License;
