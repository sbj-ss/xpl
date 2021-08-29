<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :define command 
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
 
  <MustSucceed name="pass/content">
    <Input>
      <Prev>prev-content</Prev>
      <xpl:define name="A">
        <B>
          <joined-txt><xpl:content select=".//text()"/></joined-txt>
          <rel-content><xpl:content select="preceding-sibling::Prev[1]/text()"/></rel-content>  
          <c-count><xpl:content select="count(C)"/></c-count>
          <attr-content><xpl:content select="@attr"/></attr-content>
        </B>
      </xpl:define>
      <A attr="attribute value">
        <C>1</C>
        <C>2</C>
        <C>3</C>
      </A>
    </Input>
    <Expected>
      <Prev>prev-content</Prev>
      <B>
        <joined-txt>123</joined-txt>
        <rel-content>prev-content</rel-content>
        <c-count>3</c-count>
        <attr-content>attribute value</attr-content>
      </B>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/nested">
    <Input>
      <xpl:define name="Outer" id="Outer">
        <ProcessedOuter>
          <xpl:define name="Inner">
            <ProcessedInner>
              <xpl:content select="@outer-attr" id="Outer"/>
            </ProcessedInner>
          </xpl:define>
          <xpl:content/>
        </ProcessedOuter>
      </xpl:define>
      <Outer outer-attr="attr">
        <Inner/>
      </Outer>
      <Inner/>
    </Input>
    <Expected>
      <ProcessedOuter>
        <ProcessedInner>attr</ProcessedInner>
      </ProcessedOuter>
      <Inner/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/redefinition">
    <Input>
      <xpl:define name="A">
        <Old/>
      </xpl:define>
      <xpl:define name="A">
        <New/>
      </xpl:define>
      <xpl:define name="A" replace="false">
        <Wrong/>
      </xpl:define>
      <A/>
    </Input>
    <Expected>
      <New/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/different-namespaces">
    <Input>
      <xpl:define name="ns-c:A" xmlns:ns-c="http://first.c.com">
        <Result>First ns: <xpl:content/></Result>
      </xpl:define>
      <xpl:define name="ns-c:A" xmlns:ns-c="http://second.c.com">
        <Result>Second ns: <xpl:content/></Result>
      </xpl:define>
      <ns-c:A xmlns:ns-c="http://first.c.com">first</ns-c:A>
      <ns-c:A xmlns:ns-c="http://second.c.com">second</ns-c:A>
    </Input>
    <Expected>
      <Result>First ns: first</Result>
      <Result>Second ns: second</Result>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/expand-once-with-content">
    <Input>
      <xpl:define name="Once" expand="once">
        <Result>
          <xpl:content/>
        </Result>
      </xpl:define>
      <Once>1</Once>
      <Once>2</Once>
    </Input>
    <Expected>
      <Result>1</Result>
      <Result>1</Result>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/mode-expand-now">
    <Input>
      <xpl:define name="B">
        <processed>outer</processed>
      </xpl:define>
      <xpl:define name="now">
        <xpl:define name="B">
          <processed>inner</processed>
        </xpl:define>
        <xpl:content/>
      </xpl:define>
      <now>
        <B/>
      </now>
      <now xpl:expand="now">
        <B/>
      </now>
    </Input>
    <Expected>
      <processed>inner</processed>
      <processed>inner</processed>
    </Expected>
  </MustSucceed>
   
  <MustSucceed name="pass/mode-expand-after">
    <Input>
      <xpl:define name="B">outer</xpl:define>
      <xpl:define name="after">
        <xpl:define name="B">inner</xpl:define>
        <xpl:content/>
      </xpl:define>
      <after xpl:expand="after">
        <B/>
      </after>
    </Input>
    <Expected>outer</Expected>
  </MustSucceed>

  <MustSucceed name="pass/mode-expand-skip">
    <Input>
      <xpl:define name="skip">skipped</xpl:define>
      <skip xpl:expand="skip">original</skip>
    </Input>
    <Expected>
      <skip>original</skip>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/unknown-namespace">
    <Input>
      <xpl:define name="heffalump:A"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/missing-name">
    <Input>
      <xpl:define/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-replace">
    <Input>
      <xpl:define name="A" replace="maybe"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-expand">
    <Input>
      <xpl:define name="A" expand="tomorrow"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-expand-mode">
    <Input>
      <xpl:define name="bad"/>
      <bad xpl:expand="partially"/>
    </Input>
  </MustFail>
  
  <Summary/>
 </Root>