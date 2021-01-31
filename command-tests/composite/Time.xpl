<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :get-elapsed-time, :sleep and :start-timer commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>

  <MustSucceed name="pass/simple">
    <Input>
      <xpl:start-timer/>
      <xpl:sleep delay="50"/>
      <xpl:define name="elapsed" expand="true">
        <xpl:get-elapsed-time/>
      </xpl:define>
      <xpl:choose>
        <xpl:when>
          <xpl:test><elapsed/> &gt;= 50 and <elapsed/> &lt; 75</xpl:test>
          <xpl:text>OK</xpl:text>
        </xpl:when>
        <xpl:otherwise>FAIL: <elapsed/></xpl:otherwise>
      </xpl:choose>
    </Input>
    <Expected>OK</Expected>
  </MustSucceed>

  <MustSucceed name="pass/no-restart">
    <Input>
      <xpl:start-timer/>
      <xpl:sleep delay="50"/>
      <xpl:define name="Hide"/>
      <Hide>
        <xpl:get-elapsed-time restartmeasurement="false"/>
      </Hide>
      <xpl:sleep delay="50"/>
      <xpl:define name="elapsed" expand="true">
        <xpl:get-elapsed-time/>
      </xpl:define>
      <xpl:if>
        <xpl:test><elapsed/> &gt;= 100 and <elapsed/> &lt; 150</xpl:test>
        <xpl:text>OK</xpl:text>
      </xpl:if>
    </Input>
    <Expected>OK</Expected>
  </MustSucceed>

  <MustFail name="fail/get-elapsed-time-bad-restart-mgmt">
    <Input>
      <xpl:get-elapsed-time restartmeasurement="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/sleep-missing-delay">
    <Input>
      <xpl:sleep/>
    </Input>
  </MustFail>

  <MustFail name="fail/sleep-bad-delay">
    <Input>
      <xpl:sleep delay="long"/>
    </Input>
  </MustFail>

  <Summary/>
</Root>