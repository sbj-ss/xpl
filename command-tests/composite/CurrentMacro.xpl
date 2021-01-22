<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :current-macro command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/simple">
    <Input>
      <xpl:define name="A">
        <xpl:content/>
      </xpl:define>
      <A>
        <xpl:current-macro/>
      </A>
    </Input>
    <Expected>A</Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/detailed">
    <Input>
      <xpl:define name="A">
        <xpl:content/>
      </xpl:define>
      <A>
        <xpl:current-macro detailed="true"/>
      </A>
    </Input>
    <Expected>
      <macro namespaceuri="" prefix="" name="A" line="23" parentname="Output" parentline="15" timesencountered="1" timescalled="1" disabledspin="0" expansionstate="expand always"/>    
    </Expected>
  </MustSucceed>

  <MustSucceed name="pass/detailed-with-custom-name">
    <Input>
      <xpl:define name="ns-a:A" expand="once">
        <xpl:content/>
      </xpl:define>
      <ns-a:A>
        <xpl:current-macro detailed="true" tagname="M"/>
      </ns-a:A>
    </Input>
    <Expected>
      <M namespaceuri="http://a.example.com" prefix="ns-a" name="A" line="37" parentname="Output" parentline="15" timesencountered="1" timescalled="1" disabledspin="0" expansionstate="awaiting expansion"/>    
    </Expected>
  </MustSucceed>
  
  <Summary/>
</Root>