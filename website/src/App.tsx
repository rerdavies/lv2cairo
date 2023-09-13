
import { Routes, Route } from "react-router-dom";
import License from "./routes/License";
import loadable from '@loadable/component';
import pMinDelay from 'p-min-delay';

import About from './routes/About';
import Navbar from './Navbar';
import NoMatch from './NoMatch';
import './App.css';



const Home = loadable(() => pMinDelay(import('./routes/Home'),200));
const Documentation = loadable(() => pMinDelay(import('./routes/Documentation'),200));


function App() {
    return (
        <div style={{ position: "absolute", top: 0, left: 0, right: 0, bottom: 0, display: "flex", flexDirection: "column", flexWrap: "nowrap", height: "100%", overflow: "hidden" }}>
            <div style={{ flex: "0 0 auto" }}>
                <Navbar />
            </div>
            <div style={{ flex: "1 1 100%", overflow: "hidden", position: "relative" }}>
                <Routes>
                    <Route path="/" element={<Home />} />
                    <Route path="/license" element={<License />} />
                    <Route path="/about" element={<About />} />
                    <Route path="/documentation/*" element={<Documentation />} />
                    <Route path="*" element={<NoMatch />} />
                </Routes>

            </div>
        </div>
    );
}

export default App;
