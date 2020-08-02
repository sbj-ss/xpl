<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :append command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl" xmlns:ns-a="http://a.example.com">
  <OwnContent>
    <Before>
      <Marker/>
      <xpl:append select="../Marker" position="before">
        <Content/>
      </xpl:append>
    </Before>

    <After>
      <Marker/>
      <xpl:append select="../Marker" position="after">
        <Content/>
      </xpl:append>
    </After>

    <First>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" position="first">
        <Content/>
      </xpl:append>
    </First>

    <FirstToEmpty>
      <Container/>
      <xpl:append select="../Container" position="first">
        <Content/>
      </xpl:append>
    </FirstToEmpty>

    <Last>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" position="last">
        <Content/>
      </xpl:append>
    </Last>

    <LastByAlias>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append destination="../Container" position="last">
        <Content/>
      </xpl:append>
    </LastByAlias>

    <LastToEmpty>
      <Container/>
      <xpl:append select="../Container" position="last">
        <ns-a:Content/>
      </xpl:append>
    </LastToEmpty>

    <LastByDefault>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container">
        <Content/>
      </xpl:append>
    </LastByDefault>
  </OwnContent>

  <SelectedContent>
    <Before>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Marker/>
      <xpl:append select="../Marker" source="../Source/*" position="before"/>
    </Before>

    <After>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Marker/>
      <xpl:append select="../Marker" source="../Source/*" position="after"/>
    </After>

    <First>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" source="../Source/*" position="first"/>
    </First>

    <Last>
      <Source>
        <Content ns-a:attr="attr-value">1</Content>
        <Content>text-content (2)</Content>
      </Source>
      <Container>
        <OldContent/>
        <OldContent/>
      </Container>
      <xpl:append select="../Container" source="../Source/*" position="last"/>
    </Last>

    <LastMultiple>
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
    </LastMultiple>
  </SelectedContent>

  <NonElements>
    <LastAttributeToEmpty>
      <Source attr-a="attr-a value"/>
      <Container/>
      <xpl:append select="../Container" source="../Source/@attr-a" position="last"/>
    </LastAttributeToEmpty>

    <LastToTextByDefault>
      <Destination>old-content</Destination>
       <xpl:append select="../Destination/text()">
         <Content>new-content</Content>
       </xpl:append>
    </LastToTextByDefault>

    <AfterText>
      <Destination>old-content</Destination>
      <xpl:append select="../Destination/text()" position="after">new-content</xpl:append>
    </AfterText>
  </NonElements>

  <Errors>
    <MissingDestination>
       <xpl:append source="../DoesNotMatter"/>
    </MissingDestination>

    <InvalidPosition>
      <xpl:append select="../DoesNotMatter" position="upside-down">Content</xpl:append>
    </InvalidPosition>

    <InvalidSelectXPath>
      <xpl:append select="..Wrong">content</xpl:append>
    </InvalidSelectXPath>

    <InvalidSourceXPath>
      <xpl:append select="../DoesNotMatter" source="..Wrong"/>
    </InvalidSourceXPath>

    <InvalidDestinationType>
      <xpl:append select="count(../Whatever)"> content</xpl:append>
    </InvalidDestinationType>

    <InvalidSourceType> 
      <xpl:append select="../DoesNotMatter" source="count(../Whatever)"/>
    </InvalidSourceType>
  </Errors>
</Root>
