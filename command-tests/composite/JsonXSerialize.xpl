<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :jsonx-serialize command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:j="http://www.ibm.com/xmlns/prod/2009/jsonx">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:jsonx-serialize>
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
      </xpl:jsonx-serialize>
    </Input>
    <Expected>{"number_\"array":[1,-2.5e6,null],"empty":"","s":"string\\a","b":true}</Expected>
  </MustSucceed>

  <MustSucceed name="pass/format">
    <Input>
      <xpl:jsonx-serialize format="true">
        <j:object>
          <j:array name="number_&quot;array">
            <j:number>1</j:number>
            <j:number>2</j:number>
            <j:null/>
          </j:array>
          <j:string name="empty"/>
          <j:string name="s">string\a</j:string>
          <j:boolean name="b">true</j:boolean>
        </j:object>
      </xpl:jsonx-serialize>
    </Input>
    <Expected>{
    "number_\"array": [
        1,
        2,
        null
    ],
    "empty": "",
    "s": "string\\a",
    "b": true
}
</Expected>
  </MustSucceed>

  <MustSucceed name="pass/skip-unknown">
    <Input>
      <xpl:jsonx-serialize>
        <j:heffalump>
          <j:number>1</j:number>
        </j:heffalump>
      </xpl:jsonx-serialize>
    </Input>
    <Expected/>
  </MustSucceed>

  <MustSucceed name="pass/allow-wrong-type">
    <Input>
      <xpl:jsonx-serialize>
        <j:number>abc</j:number>
      </xpl:jsonx-serialize>
    </Input>
    <Expected>abc</Expected>
  </MustSucceed>

  <MustFail name="fail/value-type-check">
    <Input>
      <xpl:jsonx-serialize valuetypecheck="true">
        <j:number>abc</j:number>
      </xpl:jsonx-serialize>
    </Input>
  </MustFail>

  <MustFail name="fail/unknown-element">
    <Input>
      <xpl:jsonx-serialize stricttagnames="true">
        <j:heffalump/>
      </xpl:jsonx-serialize>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-value-type-check">
    <Input>
      <xpl:jsonx-serialize valuetypecheck="relaxed"/>
    </Input>
  </MustFail>

  <MustFail name="fail/bad-strict-tag-names">
    <Input>
      <xpl:jsonx-serialize stricttagnames="HNTR"/>
    </Input>
  </MustFail>

  <MustFail name="fail/sequential/atoms">
    <Input>
      <xpl:jsonx-serialize>
        <j:number>1</j:number>
        <j:number>2</j:number>
      </xpl:jsonx-serialize>
    </Input>
  </MustFail>

  <MustFail name="fail/number-too-big">
    <Input>
      <xpl:jsonx-serialize valuetypecheck="true">
        <j:number>1e100500</j:number>
      </xpl:jsonx-serialize>
    </Input>
  </MustFail>

  <Summary/>
</Root>