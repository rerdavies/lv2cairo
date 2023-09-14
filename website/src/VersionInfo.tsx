

const lv2cVersion_ : string = "0.0.0-prerelease";
export function Lv2cVersion(): string { return lv2cVersion_; };

export function Lv2cVersionNotice() : string { return `
In this pre-release version, LVTK APIs are still undergoing heavy revision while
LVTK is being integrated into the Toob Amp project.

Feel free to look araound; but it would be wise to wait a week or so for things to 
settle down before using this project.
`;
}

const copyrightText: string = "Copyright 2023 Robin Davies.";

export function Lv2cCopyrightText() : string { return copyrightText; }

