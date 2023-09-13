import React from 'react';
import Typography from '@mui/material/Typography';
import Box from '@mui/material/Box';

export function Page(props: React.PropsWithChildren<{}>)
{
    return (<Box className={"page_column"}>{props.children}</Box>);
}
export function Code(props: React.PropsWithChildren<{}>)
{
    return (<span className="code">{props.children}</span>);
}

export function Heading(props: React.PropsWithChildren<{}>)
{
    return (<Typography variant="h3" paragraph>{props.children}</Typography>);
}
export function Subheading(props: React.PropsWithChildren<{}>)
{
    return (<Typography variant="h6" paragraph>{props.children}</Typography>);
}
export function P(props: React.PropsWithChildren<{}>)
{
    return (<Typography paragraph>{props.children}</Typography>);
}
export function Ul(props: React.PropsWithChildren<{}>)
{
    return (<Typography component="ul" paragraph>{props.children}</Typography>);
}

export function SmallPrint(props: React.PropsWithChildren<{}>)
{
    return (<Typography paragraph style={{fontSize: "0.8em"}}>{props.children}</Typography>);
}
export function FigureCaption(props: React.PropsWithChildren<{}>)
{
    return (<Typography paragraph style={{fontSize: "0.8em", textAlign: "center"}}>{props.children}</Typography>);
}
