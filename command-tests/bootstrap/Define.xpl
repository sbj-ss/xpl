<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :define command and :content pseudocommand
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:define name="A" id="A">
    <xpl:define name="B">
      <xpl:content select="@a" id="A"/>
    </xpl:define>
    <Processed>
      <xpl:content/>
    </Processed>
  </xpl:define>

  <A a="1">
    <B/>
  </A>
</Root>
