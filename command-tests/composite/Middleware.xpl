<?xml version="1.0" encoding="utf-8"?>
<!--
  XPL interpreter high-level test suite
  :add-middleware, :change-middleware, :clear-middleware, :get-middleware and :remove-middleware commands
-->
<Root xmlns:xpl="urn:x-xpl:xpl" xmlns:ns-a="http://a.example.com">
  <xpl:include select="/Root/xpl:define" file="Helpers.xpl"/>
  
  <MustSucceed name="pass/add">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:clear-middleware/>
      <xpl:add-middleware regex="/a/.*" file="a.xpl"/>
      <AddedA>
        <xpl:get-middleware/>
      </AddedA>
      <xpl:add-middleware regex="/a/.*" file="new-a.xpl" modifyifexists="true"/>
      <ChangedA>
        <xpl:get-middleware/>
      </ChangedA>
      <xpl:clear-middleware/>
    </Input>
    <Expected>
      <AddedA>
        <middleware regex="/a/.*" file="a.xpl"/>
      </AddedA>
      <ChangedA>
        <middleware regex="/a/.*" file="new-a.xpl"/>
      </ChangedA>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/change">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:clear-middleware/>
      <xpl:change-middleware regex="/a/.*" file="a.xpl" addifnotexists="true"/>
      <AddedA>
        <xpl:get-middleware/>
      </AddedA>
      <xpl:change-middleware regex="/a/.*" file="new-a.xpl"/>
      <ChangedA>
        <xpl:get-middleware/>
      </ChangedA>
      <xpl:clear-middleware/>
    </Input>
    <Expected>
      <AddedA>
        <middleware regex="/a/.*" file="a.xpl"/>
      </AddedA>
      <ChangedA>
        <middleware regex="/a/.*" file="new-a.xpl"/>
      </ChangedA>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/remove">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:clear-middleware/>
      <xpl:add-middleware regex="/a/.*" file="a.xpl"/>
      <xpl:add-middleware regex="/b/.*" file="b.xpl"/>
      <Added>
        <xpl:get-middleware/>
      </Added>
      <xpl:remove-middleware regex="/a/.*"/>
      <xpl:remove-middleware regex="/c/.*" ignoreifnotexists="true"/>
      <RemovedA>
        <xpl:get-middleware/>
      </RemovedA>
      <xpl:clear-middleware/>
      <Cleared>
        <xpl:get-middleware/>
      </Cleared>
    </Input>
    <Expected>
      <Added>
        <middleware regex="/a/.*" file="a.xpl"/>
        <middleware regex="/b/.*" file="b.xpl"/>
      </Added>
      <RemovedA>
        <middleware regex="/b/.*" file="b.xpl"/>        
      </RemovedA>
      <Cleared/>
    </Expected>
  </MustSucceed>
  
  <MustSucceed name="pass/get">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:clear-middleware/>
      <xpl:add-middleware regex="/a/.*" file="a.xpl"/>
      <SingleA>
        <xpl:get-middleware regex="/a/.*"/>
      </SingleA>
      <SingleC>
        <xpl:get-middleware regex="/c/.*"/>
      </SingleC>
      <AllInNs xmlns:ns-b="http://b.com">
        <xpl:get-middleware tagname="ns-b:mw"/>
      </AllInNs>
      <xpl:define name="middleware">
        <Changed/>
      </xpl:define>      
      <Repeat>
        <xpl:get-middleware/>
      </Repeat>
      <NoRepeat>
        <xpl:get-middleware repeat="false"/>
      </NoRepeat>
      <xpl:clear-middleware/>
    </Input>
    <Expected>
      <SingleA>a.xpl</SingleA>
      <SingleC/>
      <AllInNs xmlns:ns-b="http://b.com">
        <ns-b:mw regex="/a/.*" file="a.xpl"/>
      </AllInNs>
      <Repeat>
        <Changed/>
      </Repeat>
      <NoRepeat>
        <middleware regex="/a/.*" file="a.xpl"/>
      </NoRepeat>
    </Expected>
  </MustSucceed>

  <MustFail name="fail/add-no-sa-mode">
    <Input>
      <xpl:add-middleware regex="/a/.*"/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-no-sa-mode">
    <Input>
      <xpl:change-middleware regex="/a/.*"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-no-sa-mode">
    <Input>
      <xpl:remove-middleware regex="/a/.*"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/get-no-sa-mode">
    <Input>
      <xpl:get-middleware/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/clear-no-sa-mode">
    <Input>
      <xpl:clear-middleware/>
    </Input>
  </MustFail>
 
  <MustFail name="fail/add-no-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-middleware/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-no-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-middleware/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-no-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-middleware/>
    </Input>
  </MustFail>

  <MustFail name="fail/add-bad-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-middleware regex="a{["/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-bad-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-middleware regex="a{["/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-bad-regex">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-middleware regex="a{["/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/add-bad-modify-if-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-middleware regex=".*" modifyifexists="maybe"/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-bad-add-if-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-middleware regex=".*" addifnotexists="probably"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-bad-ignore-if-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-middleware regex=".*" ignoreifnotexists="if the moon is full"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/add-already-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:add-middleware regex=".*"/>
      <xpl:add-middleware regex=".*"/>
      <xpl:clear-middleware/>
    </Input>
  </MustFail>

  <MustFail name="fail/change-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:change-middleware regex=".*"/>
    </Input>
  </MustFail>

  <MustFail name="fail/remove-not-exists">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:remove-middleware regex="/a/.*"/>
    </Input>
  </MustFail>
  
  <MustFail name="fail/get-bad-repeat">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:get-middleware repeat="every 3rd"/>
    </Input>
  </MustFail>

  <MustFail name="fail/get-bad-tagname">
    <Input>
      <xpl:set-sa-mode password="1111111"/>
      <xpl:get-middleware tagname="#Z"/>
    </Input>
  </MustFail>
  
  <Summary/>
</Root>