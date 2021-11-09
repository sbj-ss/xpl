<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :rename command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/without-ns">
    <Input>
      <ns-a:A>
        <xpl:rename select="parent::*" newname="B"/>
      </ns-a:A>
    </Input>
    <Expected>
      <B/>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/own-ns">
    <Input>
      <Outer>
        <A/>
        <Wrapper xmlns:ns-b="http://b.com">
          <A/>
          <xpl:rename select="preceding::A" newname="ns-b:B"/>
        </Wrapper>
      </Outer>
    </Input>
    <Expected>
      <Outer>
        <ns-b:B xmlns:ns-b="http://b.com"/>
        <Wrapper xmlns:ns-b="http://b.com">
          <ns-b:B/>
        </Wrapper>
      </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/invalid-node-type">
    <Input>
      <A>1</A>
      <xpl:rename select="../A/node()" newname="Q"/>
    </Input>
    <Expected>
      <A>1</A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="B">
        <Processed/>
      </xpl:define>
      <xpl:rename select="./*" newname="B">
        <A/>
      </xpl:rename>
      <xpl:rename select="./*" newname="B" repeat="true">
        <A/>
      </xpl:rename>
    </Input>
    <Expected>
      <B/>
      <Processed/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/missing-select">
    <Input>
      <xpl:rename newname="A"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/missing-new-name">
    <Input>
      <xpl:rename select="A"/>
    </Input>
  </MustFail>
 
  <MustFail name="fail/bad-select">
    <Input>
      <xpl:rename select="))Z" newname="A"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-new-name">
    <Input>
      <xpl:rename select="A" newname="#"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:rename select="A" newname="B" repeat="twice"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>