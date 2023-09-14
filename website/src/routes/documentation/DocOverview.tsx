import Box from "@mui/material/Box";
import Typography from "@mui/material/Typography";
import ArchitectureDiagram from "../../ArchitectureDiagram";
import { PreformatedText } from "../../CodeFragment";
import { Link} from "react-router-dom";
import { P, Ul } from "./StandardFormats";



function DocOverview() {
    return (
        <Box className={"page_column"}>
            <Typography variant="h3" paragraph>Overview</Typography>

            <P>
                The purpose of this section is to provide a quick tour of the major features of the LVtk library. Each of the topics in this section is
                dealt with again in more substantial detail in the developer's guide.
            </P>
            <P>
                A basic implemention of an LV UI plugin (exclusive of time spent implementing 
                the audio plugin) is exceptionally easy: about 40 lines of 
                code, and about 15 minutes of work. You don't need to know a whole lot about the lvtk 
                libary to create a basic plugin. You may want to refer to 
                the <Link to="BuildingAnLv2Plugin">Building an LV2 Plugin</Link> section 
                of the documentation to get a sense of what the basic procedure is, and decide from 
                there how much you really want to know about the full lvtk library.
            </P>

            {/**************************************** */}
            <Typography variant="h6" paragraph>What is LVtk?</Typography>

            <P>
                <i>LVtk</i> is a C++ class library for building LV2 plugins with custom user interfaces, using Cairo/X11 as a graphics backend.</P>

            <ArchitectureDiagram />
            <P><i>LVtk</i>LVtk provides the folowing features:
            </P>
            <Ul>
                    <li><i>LVtk</i> elements provide a set of widgets with layout, styling and theming that render user interfaces on a Cairo/X11 surface.
                    </li>
                    <li>The <i>Lv2ControlView</i> element provides out-of-the-box rendering of LV2 ports (control variables for an LV2 plugin) for
                        most standard port variants: dials, dropdown listboxes, toggles, on/off switches, VU meters, LEDs, status text, and port groups.
                    </li>
                    <li>The <i>Lv2UI</i> class is a base-class for implementing LV2 UI plugins.
                    </li>
                    <li>The <i>Lv2Plugin</i> class is a base-class for implementing LV2 audio plugins.</li>
            </Ul>
            <P>
                The audio plugin and UI plugin implementations are completely independent. If you have an existing
                LV2 audio plugin (or prefer to use another library), LVtk is still an entirely suitable choice for building a user interface for your plugin.
            </P>

            {/****************************************************/}
            <Typography paragraph variant="h6">
                The Lv2UI Base Class
            </Typography>

            <P>
                The Lv2UI class is the bass class for all LV2 UI plugin implementations. It deals with the following issues on your behalf:
            </P>
            <Ul>
                <li>Making the plugin visible to LV2 hosts, and dealing with host interactions.</li>
                <li>Creating and managing the X11 window into which the UI is rendered.</li>
                <li>Rendering the view tree of the plugin's interface.</li>
            </Ul>
            <P>
                Lv2UI needs a description of the ports on the LV2 audio plugin. This information
                is generated using a command-line utility, <span className={"code"}>generate_lvtk_plugin_info</span>, which is provided as
                part of the LVtk package.
            </P>
            <P>
                Lv2UI will render a serviceable user interface for an LV2 UI plugin without 
                any addition work. However, it provides features that allow 
                customization of the generated user-interface.
            </P>
            <Ul>
                <li>A theming and styling system allows easy customization of rendered UI components.</li>
                <li>A class factory system allows more ambitious replacements of default UI components, or 
                    integration of custom UI components.</li>
                <li>Or plugins can elect to render a fully-custom visual tree using LVtk elements.
                </li>
            </Ul>
            {/****************************************************/}
            <Typography paragraph variant="h6">
                The Lv2Plugin Base Class
            </Typography>
            <P>
                The Lv2Plugin class serves as a base class for LV2 audio plugins. It provides 
                a C++ wrapper around the LV2 audio plugin C APIs.
            </P>
            <P>
                Use of the Lv2Plugin class is completely optional. Lv2UI is completely isolated 
                from the LV2PLugin, so it works fine whether the audio plugin it targets is written 
                using the Lv2Plugin class, or is written using another framework. 
            </P>
            <P>
                However, if you are writing a plugin from scratch, Lv2PLugin is a good starting point. 
                It isolates you from much of the pain and tedium of the native LV2 apis.
            </P>

            {/****************************************************/}
            <Typography variant="h6" paragraph>LVtk Elements</Typography>

            <P>
                The LVtk Elements library provides graphical layout and rendering on 
                X11/Cairo surfaces.
            </P>
            <P>LvtkElements are similar in concept to HTML elements.
                They provide both layout and rendering. They also have a style system that's very loosely similar to HTML css.
            </P>
            {PreformatedText(
                `
LvtkElement::ptr Render() {
    auto stackElement 
        = LvtkVerticalStackElement::Create();

    stackElement->Style()
         .BorderWidth(1)
         .BorderColor("#E0E0E0")
         .Color("#802020")
         .Margin({4,8,4,8})
         .Padding(8);

    {
        auto typography 
            = LvtkTypographyElement::Create();

        typography->Style()
            .FontStyle(LvtkFontStyle::Italic);
        typography->Text("Hello world!");
        
        stackElement.AddChild(typography);
    }
    return stackElement;
}
`)}
            <P>
                There are a few things that are worth pointing out in this code fragment.
            </P>
            <P>
                All LVtk elements inherit from the  <span className="code">LvtkElement</span> class. The lifetime of
                LVtk Elements is managed using <span className="code">std::shared_ptr&lt;&gt;</span> pointers. Each element
                has a <span className="code">Create</span> method that returns
                a <span className="code">shared_ptr</span>. <span className="code">LvtkVerticalStackElement::Create()</span>, for example, has a return type
                of <span className="code">LvtkVerticalStackElement::ptr</span>, which is equivalent to <span className="code">std::shared_ptr&lt;Lv2kVerticalStackElement&gt;</span>.
            </P>
            <P>
                Every <span className="code">LvtkElement</span> has a style object, which is conceptually similar (but not slavishly identical) to a CSS style. An object's
                style is accessible via the <span className="code">LvtkElement::Style()</span> method. Like CSS, some style attributes are inherited from parent objects, and
                some are not. In the example above, the <span className="code">LvtkStyle::Color</span> attribute set on the parent container is inherited by
                the <span className="code">LvtkTypographyElement</span>.
            </P>
            <P>
                LVtk's layout system borrows from the HTML box model. Each element has a margin,
                border, and padding that are used to calculate layout. Child elements are placed entirely within the client area
                 of a LvtkElement (the area left over after
                margins, border width and padding are removed). The same is true when drawing: when drawing in an LvtkElement's OnDraw method, the point (0,0) is located 
                at the top left corner of the client area. Unlike HTML, percentage measurements
                are taken with respect to the available client area, (as opposed to HTML in which percentage values are calculated with respect to the closest parent with relative
                or absolute positioning). In theory, percentage values in margin, borderwidth and padding properties are calculated relative to the client area of the element's
                parent; but this is largely untested, and isn't a huge priority at present, so the behaviour should -- for practical purposes -- be considered to be undefined.
            </P>
            <P>
                A single style can be shared by multiple objects (loosely comparable to a CSS class).
            </P>
            {PreformatedText(
                `
auto style = LvtkStyle::Create();
style->HorizontalAlignment(LvtkAlignment::Stretch)
    .TextAlignment(LvtkAlignment::Center)
    .FontFamily("Piboto Condendsed")
    ;

auto element1 = LvtkTypographyElement::Create();
auto element1 = LvtkTypographyElement::Create();
element1->AddClass(style);
element1->AddClass(style);
`)}
            <P>
                This feature is particularly useful when styles are stored in a LvtkTheme.
            </P>

            <P>
                Resolution of style attributes is similar to HTML. When fetching a style attribute from a LvtkStyle object, LvtkStyle first checks to see if the attribute is set
                on itself; and if not set, checks each of the class styles on its parent element, and then (if it is an inherited attribute)
                repeats the same operation each of the element's parents until a style is found on which the attribute is set.
            </P>
            {/****************************************************/}
            <Typography variant="h6" paragraph>
                LvtkElement Elements
            </Typography>
            <P>
                The LvtkElement library provides the following Container elements.
                <ul>
                    <li>LvtkContainerElement: the base class for all containers. By default, child elements are stacked one on top of another.</li>
                    <li>LvtkVerticalStackElement: child elements stack vertically.</li>
                    <li>LvtkHorizontalStackElement: child elements stack horizontally.</li>
                    <li>LvtkFlexGridElement: similar to a HTML flex-grid layout.</li>
                    <li>LvtkDropShadowElement: renders a drop-shadow under all child elements, or an inset shadow over all child elements.</li>
                </ul>
            </P>
            <P>
                The LvtkElement library provides the following UI controls.
            </P>
            <Ul>
                <li>LvtkDialElement: A knob control.</li>
                <li>LvtkPngDialElement: A knob control that uses a pre-rendered PNG animation strip.</li>
                <li>LvtkSvgDialElement: A rotating knob control that uses a PNG image.</li>
                <li>LvtkEditBoxElement: An edit box.</li>
                <li>LvtkNumericEditBoxElement: An edit box for editing numeric values, with validation and range-checking.</li>
                <li>LvtkDropdownElement: An dropdown listbox control.</li>
                <li>LvtkSwitchElement: A switch</li>
                <li>LvtkOnOffSwitchElement: A switch that is grayed out in OFF state.</li>
                <li>LvtkVuElement: a customizable VU meter.</li>
                <li>LvtkLedElement: an LED lamp.</li>
            </Ul>
            <P>
                The LvtkElement library provides the following elements for displaying content:
            </P>
                <Ul>
                    <li>LvtkTypographyElement: display text.</li>
                    <li>LvtkSvgElement: Render an SVG file to the display.</li>
                    <li>LvtkPngElement: Display a PNG file to the display.</li>
                    <li>LvtkElement: The base class for all Lvtk elements. Draws a background (if any), and borders (if any) by default.</li>
                </Ul>

            {/****************************************************/}
            <Typography variant="h6">
                LvtkElement Binding Properties
            </Typography>
            <P>
                The LvtkElement library uses a property-binding mechanism for binding display values of LvtkElements to
                values in object models. Property binding is implemented by the LvtkBindingSite&gt;T&lt; class. When two or more
                properties are bound together, setting the value of one property will change the value of the others.
            </P>
            {PreformatedText(
                `
BindingSite<double> sourceProperty { 1.0};
BindingSite<double> targetProperty { 0.0};

sourceProperty.Bind(targetProperty); // properties are now bound. 

assert(sourceProperty.get() == 1.0); // both properties have
assert(targetProperty.get() == 1.0);  // sourceProperty's value. 

sourceProperty.set(2.0);

assert(sourceProperty.get() == 2.0); 
assert(targetProperty.get() == 2.0); // the same value!.

targetProperty.set(3.0);
assert(sourceProperty.get() == 3.0); 
assert(targetProperty.get() == 3.0); // the same value!.

`)}

            <P>
                LvtkBindingSite properties are also observable: it's possible to register a callback that will
                be called whenever the value of the property changes.
            </P>
            {PreformatedText(
                `
BindingSite<double> sourceProperty { 1.0};
{
    binding_handle_t bindingHandle = 
        sourcePropert.addObserver([this](const double&value) {
            this->OnSourceChanged(value);
        });

    sourceProperty.set(2.0); // the callback gets called.
} // bindingHandle goes out of scope.
  // the observer is removed!


`)}
            <P>
                Be very careful with the returned binding handle. The returned handle is special! The observer gets
                removed when the binding_handle_t is destroyed!  binding_handle_t's are moveable (but not copyable), so you
                can safel move the binding_handle_t if you have to. The arrangement is reciprocal: if the observable gets
                destroyed, the link between the overvation_handle_t and the observable gets broken as well. The binding_handle_t
                and the observable can be safely destroyed in either order. This avoids the all-too-common mistakes of trying to
                notify an observer that has been destroyed, or trying to remove an observer from an observable that has been destroyed. But it
                comes at a slight cost. Just remember: if you're not getting the callback you're expecting, you probably forgot to
                save the the binding_handle_t somewhere safe.
            </P>
            <P>
                Consider the  example of how the Lv2UI class connects port values in the LV2 audio plugin with port values in the user interface. The Lv2UI class
                exposes LV2 port values as bindable properties (Lv2UI::getPortProperty()). New values that are received from the host are set on the
                port properties, and any values that have been bound to the Lv2UI port values will be updated as well. If <span className="code">set()</span> gets called on
                a Lv2UI port value (or any property that has been bound to it), Lv2UI will send the new value to the LV2 host.
            </P>
            <P>
                When an Lv2ControlView is created, it ValueProperty of the Lv2ControlView is bound to the port value property on the owning Lv2UI property.
                An internal Lv2ControlModel object observes the LV2ControlView's Value property, and if it detects a change, it updates properties on
                the Lv2ControlModel -- the DialValueProperty, and DisplayValueProperty most notably -- based on the properties of the Lv2PortInfo for
                the port in question. Appropriate mappings are made for MinValue, MaxValue, IsLogarithmic, IsEnumerated, IsInteger, and other Lv2PortInfo properties, as
                well as for the display values settings such as Lv2PortInfo::Units().
            </P>
            <P>
                These properties will have been bound to Properties on LvtkElement objectes in the view tree. So when Lv2PortModel updates its internal properties,
                the dial position and display values will change appropriately.
            </P>
            <P>
                The process works in reverse as follows. The rendered LvtkDialElement updates its ValueProperty. This updates the corresponding property on Lv2ViewModel.
                Lv2ViewModel maps the dial value based on Lv2PortInfo settings, and sets its ValueProperty. This property will have been bound to the Lv2PortView element's
                ValueProperty, which in turn has been bound to the Lv2UI's port property. Lv2UI detects the change, and sends the new value to the host.
            </P>
            <P>
                If that's not clear, don't worry. There are simpler examples in the Developer's Guide section on data binding. From an overview perspective,
                what you need to know is that the LvtkElement library is designed to use property binding, and uses property binding internally
                to bind port properties on the host to values in the rendered display. (And yes, I know, Lv2PortModel is really a ViewController. But that's
                probably too abstruse a point to deal with here).
            </P>
            <Typography variant="h6">
                Themes
            </Typography>
            <P>
                The LvtkTheme object provides colors, styles and other attributes that are used to control themed rendering.
                You can customize a theme, but setting a new theme on the root LvtkWindow (Lv2UI::Window()), or setting
                a theme on any LvtkElement in the view tree (LvtkElement::Theme(LvtkTheme::ptr)).
                The current theme is accessible via the LvtkElement::Theme() property. If not set, Theme() inherits the
                theme of its parent element. And if no element has a set theme, LvtkWindow::Theme() is used. This allows themes
                to be swapped at any point in the view tree, by calling LvtkElement::Theme(). After calling LvtkElement::Theme(), the current
                element and all its children will use the new theme.
            </P>
            <P>
                If you are implementing themed custom controls that need theme values beyond the standard theme properties, LvtkTheme
                provides dictionaries of colors and styles that are keyed by a string value. See LvtkTheme::setColor, and LvtkTheme::Style.
            </P>
            <P>
                Note that Theme() cannot be called before an element is mounted. If you need to calculate values based on theme values,
                you should do those calculations in an override of LvtkElement::OnMount().
            </P>
            <P>
                An obvious customization point would be to set LvtkTheme::paperColor, which will change the background of the UI.
            </P>

            <Typography paragraph variant="h6">
                The Lv2ControlView
            </Typography>

            <P>
                The Lv2ControlView provides a widget that displays appropriate controls for an LV2 port (a property value
                for the audio plugin). An Lv2ControlView selects approprate controls for an LV2 port (dial, drop-down selector,
                toggle, VU meter, LED lamp, &c), and manages binding display values with actual plugin values.
            </P>

            <P>
                The Lv2ControlView element takes a reference to a LvtkBindingSite (nominally, the property for the LV2 Port provided by Lv2UI), and
                an Lv2PortInfo object that encapsulates all of the properties of a port object as captured from its TTL description (more on this
                later). Lv2ControlView manages rendering of the controls, as well of binding of the port property with controls in the generated view tree.
            </P>
            <P>
                It is the nature of great peice of software that they make the complicated very simple. Suffice it to say that there's a substantial
                amount of logic involved in dealing with all the various configurable poperties of an LV2 port object. You may, in fact, not even
                have to deal with it directly. But if you are building radically custom user interfaces, this is an important building block.
            </P>
            <P>
                In the event that Lv2ControlView does not capture your needs fully, there are a few virtual methods that allow some customization.
                Refer to the developer documentation for details.
            </P>


            {/****************************************************/}
            <Typography variant="h6" paragraph>
                About Lv2PLuginInfo
            </Typography>
            <P>
                The Lv2PLuginInfo class encapsulates information about a plugin as captured from its <span className={"code"}>manifest.ttl</span> file.
                LvwUI needs this information in order to generate views of the port controls. Crucially, Lv2PluginInfo contains information for
                all of the lv2:port declarations in a plugin ttl, plus supplementary information about properties that can be loaded from a file dialog.
            </P>
            <P>
                Rather than parsing TTL files at runtime, Lv2UI relies on a command-line utility to parse TTL files, and generate a C++ header file
                that captures a description of all the information in the TTL files.  This header file is then incorporated into the build for
                a an LV2 UI plugin.
            </P>
            <P>
                The <span className="code">generate_lvtk_plugin_info</span> command-line utility generates the necessary header file, using either
                information from a currently-installed LV2 audio plugin, using .ttl files found in <span className="code">/usr/bin/lv2</span>, or
                from a <span className="code">manifest.ttl</span> file provided as an argument.
            </P>``
            <P>

                The <span className="code">generate_lvtk_plugin_info</span> executable is generated by the LVtk project build procedure. The SamplePlugin project
                demonstrates how to automate the build procedure for the Lv2PluginInfo file in a CMake project.
            </P>
            <P>
                The developer guide deals with the procedure for capturing Lv2PLuginINfo headers in more detail. From the perspective of an overview,
                what you need to know is that Lv2PluginInfo information plays a crucial role in the development of an LV2 UI plugin. Lv2UI makes extensive
                use of this information. Lv2PluginInfo declarations are generated by a command-line utility from the relevant .TLL files for a plugin.
            </P>

        </Box>
    );
}

export default DocOverview;