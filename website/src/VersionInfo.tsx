

const lv2cVersion_ : string = "0.0.0-prerelease";
export function Lv2cVersion(): string { return lv2cVersion_; };

export function Lv2cVersionNotice() : string { return `Please be advised that LVTK
APIs are undergoing heavy revision as LVTK is being used to implement native 
user interface for the ToobAmp project. 

If LVTK is something you are interested in, it would be wise to wait for a week or two
before starting to incorporate it into any of your projects.

`;
}

const copyrightText: string = "Copyright 2023 Robin Davies.";

export function Lv2cCopyrightText() : string { return copyrightText; }

