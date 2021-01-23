<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :isolate command
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <A>Outer</A>
  <xpl:isolate>
    <xpl:no-expand>
      <A>Inner</A>
      <xpl:include select="//A"/>
    </xpl:no-expand>
  </xpl:isolate>
</Root>