<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :append command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="own-content/before">
    <Input>
      <Marker/>
      <xpl:append select="../Marker" position="before">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Content/>
      <Marker/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/after">
    <Input>
      <Marker/>
      <xpl:append select="../Marker" position="after">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Marker/>
      <Content/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/first">
    <Input>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" position="first">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <Content/>
        <OldContent/>
        <OldContent/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/first-to-empty">
    <Input>
      <Container/>
      <xpl:append select="../Container" position="first">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <Content/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/last">
    <Input>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" position="last">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <OldContent/>
        <OldContent/>
        <Content/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/last-by-alias">
    <Input>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append destination="../Container" position="last">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <OldContent/>
        <OldContent/>
        <Content/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/last-to-empty">
    <Input>
      <Container/>
      <xpl:append select="../Container" position="last">
        <ns-a:Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <ns-a:Content/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="own-content/last-by-default">
    <Input>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container">
        <Content/>
      </xpl:append>
    </Input>
    <Expected>
      <Container>
        <OldContent/>
        <OldContent/>
        <Content/>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="selected-content/before">
    <Input>
      <Source>
        <Content>1</Content>
      </Source>
      <Marker/>
      <xpl:append select="../Marker" source="../Source/*" position="before"/>
    </Input>
    <Expected>
      <Source>
        <Content>1</Content>
      </Source>
      <Content>1</Content>
      <Marker/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="selected-content/after">
    <Input>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
      </Source>
      <Marker/>
      <xpl:append select="../Marker" source="../Source/*" position="after"/>
    </Input>
    <Expected>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
      </Source>
      <Marker/>
      <Content ns-a:attr="attr-value">1</Content>
    </Expected>
  </MustSucceed>

  <MustSucceed name="selected-content/first">
    <Input>
      <Source>
        <Content>1</Content>
      </Source>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" source="../Source/*" position="first"/>
    </Input>
    <Expected>
      <Source>
        <Content>1</Content>
      </Source>
      <Container>
        <Content>1</Content>
        <OldContent/>
        <OldContent/>
      </Container>
    </Expected>
  </MustSucceed>   

  <MustSucceed name="selected-content/last">
    <Input>
      <Source>
        <Content>1</Content>
      </Source>
      <Container>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" source="../Source/*" position="last"/>
    </Input>
    <Expected>
      <Source>
        <Content>1</Content>
      </Source>
      <Container>
        <OldContent/>
        <Content>1</Content>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="selected-content/last-multiple">
    <Input>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" source="../Source/*" position="last"/>
    </Input>
    <Expected>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Container>
        <OldContent/>
        <OldContent/>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Container>
      <Container>
        <OldContent/>
        <OldContent/>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="non-element/last-attribute-to-empty">
    <Input>
      <Source attr-a="attr-a value"/>
      <Container/>
      <xpl:append select="../Container" source="../Source/@attr-a" position="last"/>
    </Input>
    <Expected>
      <Source attr-a="attr-a value"/>
      <Container>attr-a value</Container>
    </Expected>
  </MustSucceed>

  <MustSucceed name="non-element/last-to-text-by-default">
    <Input>
      <Destination>a</Destination>
       <xpl:append select="../Destination/text()">
         <Content>b</Content>
       </xpl:append>
    </Input>
    <Expected>
      <!-- text node is not a container, appending last element to it fails -->
      <Destination>a</Destination>
    </Expected>
  </MustSucceed>

  <MustSucceed name="non-element/after-text">
    <Input>
      <Destination>a</Destination>
      <xpl:append select="../Destination/text()" position="after">b</xpl:append>
    </Input>
    <Expected>
      <Destination>ab</Destination>
    </Expected>
  </MustSucceed>

  <MustFail name="errors/missing-destination">
    <Input>
      <xpl:append source="../DoesNotMatter"/>
    </Input>
  </MustFail>

  <MustFail name="errors/invalid-position">
    <Input>
      <xpl:append select="../DoesNotMatter" position="upside-down">Content</xpl:append>
    </Input>
  </MustFail>

  <MustFail name="errors/invalid-select-xpath">
    <Input>
      <xpl:append select="..Wrong">content</xpl:append>
    </Input>
  </MustFail>

  <MustFail name="errors/invalid-source-xpath">
    <Input>
      <xpl:append select="../DoesNotMatter" source="..Wrong"/>
    </Input>
  </MustFail>

  <MustFail name="errors/invalid-destination-type">
    <Input>
      <xpl:append select="count(../Whatever)"> content</xpl:append>
    </Input>
  </MustFail>

  <MustFail name="errors/invalid-source-type">
    <Input>
      <xpl:append select="../DoesNotMatter" source="count(../Whatever)"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>
