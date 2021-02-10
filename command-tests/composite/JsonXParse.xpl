<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :jsonx-parse command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:j="http://www.ibm.com/xmlns/prod/2009/jsonx">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:jsonx-parse>{"number_\"array":[1,-2.5e6,null],"empty":"","s":"string\\a","b":true}</xpl:jsonx-parse>
    </Input>
    <Expected>
      <j:object>
        <j:array name="number_&quot;array">
          <j:number>1</j:number>
          <j:number>-2.5e6</j:number>
          <j:null/>
        </j:array>
        <j:string name="empty"/>
        <j:string name="s">string\a</j:string>
        <j:boolean name="b">true</j:boolean>
      </j:object>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-validate">
    <Input>
      <xpl:jsonx-parse validate="lazily"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:jsonx-parse repeat="quantum satis"/>
    </Input>
  </MustFail>

  <MustFail name="fail/torn-input">
    <Input>
      <xpl:jsonx-parse>{"a":1,</xpl:jsonx-parse>
    </Input>
  </MustFail>

  <MustFail name="fail/invalid-input">
    <Input>
      <xpl:jsonx-parse>{{</xpl:jsonx-parse>
    </Input>
  </MustFail>
 
  <Summary/>
</Root>