<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :text command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <Outer>
        <xpl:text>  <A/>  </xpl:text>
      </Outer>
    </Input>
    <Expected>
      <Outer>  <A/>  </Outer>
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <ProcessedA/>
      </xpl:define>
      <xpl:text>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:text>
      <xpl:text repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:text>
    </Input>
    <Expected>
      <A/>
      <ProcessedA/>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:text repeat="maybe"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>