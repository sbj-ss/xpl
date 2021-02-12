<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :fatal command
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/single-child">
    <Input>
      <Header/>
      <xpl:fatal>
        <Root>
          <ns-a:error>failed</ns-a:error>
        </Root>
      </xpl:fatal>
      <Trailer/>
    </Input>
    <!-- note that :isolate eats document root -->
    <Expected>
      <ns-a:error>failed</ns-a:error>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/multiple-children">
    <Input>
      <Header/>
      <xpl:fatal>
        some text
        <A/>
        <B/>
        <C/>
      </xpl:fatal>
      <Trailer/>
    </Input>
    <Expected/>
  </MustSucceed>
  
  <MustSucceed name="pass/no-child-elements">
    <Input>
      <Header/>
      <xpl:fatal/>
      <Trailer/>
    </Input>
    <Expected>fatal command called</Expected>
  </MustSucceed>
  
  <Summary/>
</Root>