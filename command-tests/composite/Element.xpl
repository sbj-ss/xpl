<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :element command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <xpl:element name="A">
        <Content/>
      </xpl:element>
    </Input>
    <Expected>
      <A>
        <Content/>
      </A>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/own-ns">
    <Input>
      <xpl:element xmlns:ns-b="http://b.com" name="ns-b:A"/>
    </Input>
    <Expected>
      <ns-b:A xmlns:ns-b="http://b.com"/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/no-repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:element name="A" repeat="false"/>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustFail name="fail/missing-name">
    <Input>
      <xpl:element/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-name">
    <Input>
      <xpl:element name="))"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:element name="A" repeat="never"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>