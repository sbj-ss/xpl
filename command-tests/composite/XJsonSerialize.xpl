<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :xjson-serialize command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:j="http://www.ibm.com/xmlns/prod/2009/jsonx">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:xjson-serialize>
        <j:object>
          <j:array name="number_&quot;array">
            <j:number>1</j:number>
            <j:number>2</j:number>
            <j:null/>
          </j:array>
          <j:string name="s">string\a</j:string>
          <j:boolean name="b">true</j:boolean>
        </j:object>
      </xpl:xjson-serialize>
    </Input>
    <Expected>{"number_\"array":[1,2,null],"s":"string\\a","b":true}</Expected>
  </MustSucceed>

  <MustSucceed name="pass/skip-unknown">
    <Input>
      <xpl:xjson-serialize>
        <j:heffalump>
          <j:number>1</j:number>
        </j:heffalump>
      </xpl:xjson-serialize>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/allow-wrong-type">
    <Input>
      <xpl:xjson-serialize>
        <j:number>abc</j:number>
      </xpl:xjson-serialize>
    </Input>
    <Expected>abc</Expected>
  </MustSucceed>

  <MustFail name="fail/value-type-check">
    <Input>
      <xpl:xjson-serialize valuetypecheck="true">
        <j:number>abc</j:number>
      </xpl:xjson-serialize>
    </Input>
  </MustFail>

  <MustFail name="fail/unknown-element">
    <Input>
      <xpl:xjson-serialize stricttagnames="true">
        <j:heffalump/>
      </xpl:xjson-serialize>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-value-type-check">
    <Input>
      <xpl:xjson-serialize valuetypecheck="relaxed"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-strict-tag-names">
    <Input>
      <xpl:xjson-serialize stricttagnames="HNTR"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>