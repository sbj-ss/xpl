<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL bootstrap test suite
  :choose, :when, :test, :otherwise commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl">
  <xpl:choose>
    <xpl:when>
      <xpl:test>1=1</xpl:test>
      <OK/>
    </xpl:when>
    <xpl:otherwise>
      <Failed/>
    </xpl:otherwise>
  </xpl:choose>

  <xpl:choose>
    <xpl:when>
      <xpl:test>/Jabberwocky</xpl:test>
      <Failed/>
    </xpl:when>
    <xpl:otherwise>
      <OK/>
    </xpl:otherwise>
  </xpl:choose>
</Root>
