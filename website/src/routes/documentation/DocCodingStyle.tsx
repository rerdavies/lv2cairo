import { Code, Heading, Subheading, Page, P, Ul } from './StandardFormats';

import { PreformatedText } from "../../CodeFragment";


function DocCodingStyle() {
    return (
        <Page>
            <Heading>Lv2Cairo Coding Style Guidlines</Heading>
            <P>These are the coding standards currently used by Lv2Cairo. Like all good coding standards,
                these are guidelines. Violate them when good taste and judgement warrants. In particular,
                Lv2Cairo coding style does vary when coding close to 3rd-party libraries. The Lv2cDrawingContext,
                for example, uses snake-case method names that mirror the naming conventions of the underlying Lv2Cairo C APIs..
            </P>
            <P>You are encouranged (but by no means required) to use these coding conventions when writing projects that
                are based on Lv2Cairo.
            </P>
            <P>At present, there may be places where an older coding standard has leaked into Lv2Cairo code. These cases are being
                dealt with as they are discovered; but this coding guideline reflects where the code should end up.
            </P>
            <Subheading >Formatting</Subheading>
            <Ul>
                <li>Indenting is 4 spaces (no tabs).</li>
                <li>Formatting follows the conventions of the Visual Studio Code C++ code formatter. Contributors are
                    encouraged to use Visual Studio Code for editing. There are things the Visual Studio code formatter
                    does that I dislike (most particularly how it handles hanging indents); but the advantage of automated
                    code formatting far outweigh the temptations of trying to improve on them. Submitted code will be
                    subjected to the standard formatter; so it's advisable to get used to working around the
                    more foolish things that the Visual Studio formatter does.
                </li>
            </Ul>
            <Subheading>Casing Conventions</Subheading>
            <Ul>
                <li>Typenames use title-case.<br /> e.g. <Code>class Lv2cRectangle.</Code></li>
                <li>Method names use title-case.<br />e.g. <Code>void RenderElements();</Code></li>
                <li>To avoid conflicts between typenames, methods and getters, it is advisable to
                    include a module-specific prefix or suffix in typenames.<br />
                    e.g. <Code>class Lv2cRectangle;  class Lv2UI;</Code>
                </li>
                <li>Parameters, variables, and class variables use lower-camel-case.
                    <br /><br />e.g: <Code>Lv2cRectangle screenBounds;</Code>
                    <br /><br />It is acceptable to use '_' as a <i>suffix</i> for variable names  where it is desirable to
                    distinguish between method names, arguments and variables. But this should be a rare
                    exception, and not a rule.
                </li>
                <li>Defines and constexprs use upper-snake-case. 
                    <br/><Code>constexpr double MAX_WIDTH = 40;</Code></li>
                <li>Acronyms are camel-cased, even if the real-world acronum is upper-cased. HtmlElement, not HTMLElement;
                    UiFramework, not UIFramework.</li>
            </Ul>
            <Subheading>Getters and Setters</Subheading>
            <P>
                Getters and setters have have no prefix, and their name should be upper-camel-case (e.g. <Code>const Lv2cRectangle& ScreenBounds() const;</Code>.
                Names of properties should be nouns or noun phrases, in order to distinguish them from methods, which should be verbs or verb
                phrases. Using gerunds or past-participles can be helpful.
            </P>
            {PreformatedText(`
bool Updated() const;
self& Updated(bool value);

bool Updating() const;
self& Updating(bool value);
`)}
            <P>The setter should return a reference to self, in order to allow
                chaining of setters.</P>

            {PreformatedText(`
std::shared_ptr<Lv2cTypographElement> typography 
    = Lv2cTypographElement::Create();

typography
    ->Text("EQ")
    .Variant(Lv2cTypographyVariant::Body2));

typography->Style()
    .Width(120)
    .Height(36)
    .TextAlignment(Lv2cTextAlignment::Center);

Lv2cStyle style = Lv2cStyle()
                      .Width(8)
                      .Height(8)
                      .Margin({8,4,8,4});
`)}

            <P>Boolean getters and setters should not start with 'Is'. Just remove 'Is', and you will end up with a perfectly serviceable
                property name. Updating(), not IsUpdating().
            </P>
            <Subheading>General</Subheading>
            <Ul>
                <li>enums should always be scoped (enum class).</li>
                <li>Code should be written for C++20.</li>
            </Ul>

            <Subheading>Conventional Class Pre-amble</Subheading>
            <P>Most classes in Lv2Cairo are referenced through <Code>std::shared_ptr&lt;&gt;</Code>s. </P>
            <P>To make this easier, custom Lv2Cairo element classes should start with the following preamble.</P>
            {PreformatedText(`
class ToobGraphElement: public Lv2cElement {
public:
     using self = ToobGraphElement;
     using super = Lv2cElement;
     using ptr = std::shared_ptr<self>;
     static ptr Create() { return std::make_shared<self>(); }
};
`)}

        </Page>
    );
}

export default DocCodingStyle;
