<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :edge command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/no-effect">
    <Input>
      <xpl:edge/>
    </Input>
    <Expected>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/default-params">
    <Input>
      <xpl:edge>
        <Content/>
      </xpl:edge>
    </Input>
    <Expected>
      <Content/>
    </Expected>
  </MustSucceed>  

  <MustSucceed name="pass/copy-from-self-to-parent">
    <Input>
      <xpl:edge type="copy">
        <Content1/>
        <Content2/>
      </xpl:edge>
    </Input>
    <Expected>
      <Content1/>
      <Content2/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/copy-from-selection-to-parent">
    <Input>
      <Source>
        <Content1/>
        <Content2/>
      </Source>
      <Source>
        <Content3/>
        <Content4/>
      </Source>
      <xpl:edge type="copy" source="../Source/*"/>
    </Input>
    <Expected>
      <Source>
        <Content1/>
        <Content2/>
      </Source>
      <Source>
        <Content3/>
        <Content4/>
      </Source>
      <Content1/>
      <Content2/>
      <Content3/>
      <Content4/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/copy-from-self-to-selection">
    <Input>
      <Destination/>
      <Destination/>
      <xpl:edge type="copy" destination="../Destination">
        <Content1/>
        <Content2/>
      </xpl:edge>
    </Input>
    <Expected>
      <Destination>
        <Content1/>
        <Content2/>
      </Destination>
      <Destination>
        <Content1/>
        <Content2/>
      </Destination>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/copy-from-selection-to-selection">
    <Input>
      <Source>
        <Content1/>
      </Source>
      <Source>
        <Content2/>
      </Source>
      <Destination/>
      <Destination/>
      <xpl:edge type="copy" source="../Source/*" destination="../Destination"/>
    </Input>
    <Expected>
      <Source>
        <Content1/>
      </Source>
      <Source>
        <Content2/>
      </Source>    
      <Destination>
        <Content1/>
        <Content2/>
      </Destination>
      <Destination>
        <Content1/>
        <Content2/>
      </Destination>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/replace-from-self-to-selection">
    <Input>
      <Destination>
        <OldContent/>
      </Destination>
      <Destination>
        <OldContent/>
      </Destination>
      <xpl:edge type="replace" destination="../Destination/*">
        <NewContent1/>
        <NewContent2/>
      </xpl:edge>
    </Input>
    <Expected>
      <Destination>
        <NewContent1/>
        <NewContent2/>
      </Destination>
      <Destination>
        <NewContent1/>
        <NewContent2/>
      </Destination>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/replace-from-empty-self-to-selection">
    <Input>
      <Destination>
        <OldContent/>
      </Destination>
      <Destination>
        <OldContent/>
      </Destination>
      <xpl:edge type="replace" destination="../Destination/*"/>
    </Input>
    <Expected>
      <Destination/>
      <Destination/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/replace-from-selection-to-selection">
    <Input>
      <Source xmlns:ns-b="http://b.com">
        <ns-b:NewContent1/>
      </Source>
      <Source xmlns:ns-b="http://b.com">
        <ns-b:NewContent2/>
      </Source>
      <Destination>
        <OldContent/>
      </Destination>
      <Destination>
        <OldContent/>
      </Destination>
      <xpl:edge type="replace" source="../Source/*" destination="../Destination/*"/>
    </Input>
    <Expected>
      <Source xmlns:ns-b="http://b.com">
        <ns-b:NewContent1/>
      </Source>
      <Source xmlns:ns-b="http://b.com">
        <ns-b:NewContent2/>
      </Source>
      <Destination>
        <ns-b:NewContent1 xmlns:ns-b="http://b.com"/>
        <ns-b:NewContent2 xmlns:ns-b="http://b.com"/>
      </Destination>
      <Destination>
        <ns-b:NewContent1 xmlns:ns-b="http://b.com"/>
        <ns-b:NewContent2 xmlns:ns-b="http://b.com"/>
      </Destination>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/try-to-replace-self">
    <Input>
      <xpl:edge type="replace" destination="."/>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/try-to-replace-ancestor">
    <Input>
      <Outer>
        <xpl:edge type="replace" destination="ancestor::Outer[1]"/>
      </Outer>
    </Input>
    <Expected>
      <Outer/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/element-from-self-to-parent">
    <Input>
      <xpl:edge type="element" name="Wrapper">
        <Content/>
      </xpl:edge>
    </Input>
    <Expected>
      <Wrapper>
        <Content/>
      </Wrapper>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/element-from-self-to-selection">
    <Input>
      <Destination/>
      <Destination/>
      <xpl:edge type="element" name="Wrapper" destination="../Destination">
        <Content/>
      </xpl:edge>
    </Input>
    <Expected>
      <Destination>
        <Wrapper>
          <Content/>
        </Wrapper>
      </Destination>
      <Destination>
        <Wrapper>
          <Content/>
        </Wrapper>
      </Destination>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/element-from-selection-to-parent">
    <Input>
      <Source>
        <NewContent1/>
      </Source>
      <Source>
        <NewContent2/>
      </Source>
      <xpl:edge type="element" name="Wrapper" source="../Source/*"/>
    </Input>
    <Expected>
      <Source>
        <NewContent1/>
      </Source>
      <Source>
        <NewContent2/>
      </Source>
      <Wrapper>
        <NewContent1/>
        <NewContent2/>
      </Wrapper>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/element-from-selection-to-selection">
    <Input>
      <Source>
        <NewContent1/>
      </Source>
      <Source>
        <NewContent2/>
      </Source>
      <xpl:edge type="element" name="Wrapper" source="../Source/*" destination="../Destination"/>
      <Destination/>
      <Destination/>
    </Input>
    <Expected>
      <Source>
        <NewContent1/>
      </Source>
      <Source>
        <NewContent2/>
      </Source>
      <Destination>
        <Wrapper>
          <NewContent1/>
          <NewContent2/>
        </Wrapper>
      </Destination>
      <Destination>
        <Wrapper>
          <NewContent1/>
          <NewContent2/>
        </Wrapper>
      </Destination>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/attribute-from-self-to-parent">
    <Input>
      <Outer>
        <xpl:edge xmlns:ns-b="http://b.com" type="attribute" name="ns-b:attr">value</xpl:edge>
      </Outer>
    </Input>
    <Expected>
      <Outer xmlns:ns-b="http://b.com" ns-b:attr="value"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/attribute-from-self-to-selection">
    <Input>
      <Destination/>
      <Destination/>
      <xpl:edge type="attribute" name="attr" destination="../Destination">value</xpl:edge>
    </Input>
    <Expected>
      <Destination attr="value"/>
      <Destination attr="value"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/attribute-from-selection-to-parent">
    <Input>
      <Outer>
        <Source>one</Source>
        <Source>two</Source>
        <xpl:edge type="attribute" name="attr" source="../Source/text()"/>
      </Outer>
    </Input>
    <Expected>
      <Outer attr="onetwo">
        <Source>one</Source>
        <Source>two</Source>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/attribute-from-scalar-to-parent">
    <Input>
      <Outer>
        <xpl:edge type="attribute" name="attr" source="2*2"/>
      </Outer>
    </Input>
    <Expected>
      <Outer attr="4"/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/attribute-from-selection-to-selection">
    <Input>
      <Source>one</Source>
      <Source>two</Source>
      <Destination/>
      <Destination/>
      <xpl:edge type="attribute" name="attr" source="../Source/text()" destination="../Destination"/>
    </Input>
    <Expected>
      <Source>one</Source>
      <Source>two</Source>
      <Destination attr="onetwo"/>
      <Destination attr="onetwo"/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-type">
    <Input>
      <xpl:edge type="rainbow"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-source">
    <Input>
      <xpl:edge source="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-destination">
    <Input>
      <xpl:edge destination="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/scalar-destination">
    <Input>
      <xpl:edge destination="2*2"/>
    </Input>
  </MustFail>

  <MustFail name="fail/attribute-from-non-text-self">
    <Input>
      <xpl:edge type="attribute" name="whatever">
        <Node/>
      </xpl:edge>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-attribute-from-non-text-selection">
    <Input>
      <Node/>
      <xpl:edge type="attribute" name="whatever" source="../Node"/>
    </Input>
  </MustFail>

  <MustFail name="fail/unneeded-name">
    <Input>
      <xpl:edge type="copy" name="Jabberwocky"/>
    </Input>
  </MustFail>

  <MustFail name="fail/attribute-without-name">
    <Input>
      <xpl:edge type="attribute"/>
    </Input>
  </MustFail>

  <MustFail name="fail/element-without-name">
    <Input>
      <xpl:edge type="element"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>