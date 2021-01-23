<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :container command
-->
<Root xmlns:xpl="http://xpl-dev.org/xpl">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <xpl:container>
        <A/>
      </xpl:container>
    </Input>
    <Expected>
      <A/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/repeat">
    <Input>
      <xpl:define name="A">
        <B/>
      </xpl:define>
      <xpl:container>
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:container>
      <xpl:container repeat="true">
        <xpl:no-expand>
          <A/>
        </xpl:no-expand>
      </xpl:container>
    </Input>
    <Expected>
      <A/>
      <B/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/do-not-return-content">
    <Input>
      <A/>
      <xpl:container returncontent="false">
        <xpl:delete select="//A"/>
        <B/>
      </xpl:container>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustFail name="fail/bad-repeat">
    <Input>
      <xpl:container repeat="not-sure"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/bad-returncontent">
    <Input>
      <xpl:container returncontent="on-fridays"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>