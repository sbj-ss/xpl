<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :for-each command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <Src>1</Src>
      <Src>2</Src>
      <xpl:for-each select="../Src">
        <A>
          <xpl:content select="./text()"/>
        </A>
      </xpl:for-each>
    </Input>
    <Expected>
      <Src>1</Src>
      <Src>2</Src>
      <A>1</A>
      <A>2</A>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/by-attributes">
    <Input>
      <Src attr="1"/>
      <Src attr="2"/>
      <xpl:for-each select="../Src/@attr">
        <A>
          <xpl:content/>
        </A>
      </xpl:for-each>
    </Input>
    <Expected>
      <Src attr="1"/>
      <Src attr="2"/>
      <A>1</A>
      <A>2</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/ns">
    <Input>
      <Outer>
        <Src xmlns:ns-b="http://b.com">
          <ns-b:Content/>
        </Src>
        <xpl:for-each xmlns:ns-c="http://c.com" select="../Src">
          <ns-c:Processed>
            <xpl:content/>
          </ns-c:Processed>
        </xpl:for-each>
      </Outer>
    </Input>
    <Expected>
      <Outer xmlns:ns-c="http://c.com">
        <Src xmlns:ns-b="http://b.com">
          <ns-b:Content/>
        </Src>     
        <ns-c:Processed>
          <ns-b:Content xmlns:ns-b="http://b.com"/>
        </ns-c:Processed>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <Src/>
      <xpl:for-each select="../Src" repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:for-each>
    </Input>
    <Expected>
      <Src/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/with-define">
    <Input>
      <Src attr="a">1</Src>
      <xpl:for-each select="../Src" id="for-each">
        <xpl:define name="A">
          <B><xpl:content select="@attr" id="for-each"/>: <xpl:content/></B>
        </xpl:define>
        <A>
          <xpl:content/>
        </A>
      </xpl:for-each>
    </Input>
    <Expected>
      <Src attr="a">1</Src>
      <B>a: 1</B>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/missing-select">
    <Input>
      <xpl:for-each/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-select">
    <Input>
      <xpl:for-each select="))Z"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:for-each select="Whatever" repeat="twice"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>