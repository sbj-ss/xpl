<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :value-of command 
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <Calc>
    <xpl:value-of select="2*2"/>
  </Calc>

  <Nodes>
    <xpl:value-of select="count(/*)"/>
  </Nodes>    
</Root>
