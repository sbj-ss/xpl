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
        <xpl:value-of select="count(./macro[@namespaceuri=''][@prefix=''][@name='A'])">
          <xpl:current-macro detailed="true"/>
        </xpl:value-of>
      </A>
    </Input>
    <Expected>1</Expected>
  </MustSucceed>

  <MustSucceed name="pass/detailed-with-custom-name">
    <Input>
      <xpl:define name="ns-a:A" expand="once">
        <xpl:content/>
      </xpl:define>
      <ns-a:A>
        <xpl:value-of select="count(./M[@prefix='ns-a'][@name='A'][@timesencountered=1][@expansionstate='awaiting expansion'])">
          <xpl:current-macro detailed="true" tagname="M"/>
        </xpl:value-of>
      </ns-a:A>
    </Input>
    <Expected>1</Expected>
  </MustSucceed>
  
  <Summary/>
</Root>