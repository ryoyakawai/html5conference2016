  Polymer({
      midiAccess:null,
      outputIdx: "false",
      pfmNow: 0,
      pitchBendRange:{"min":0, "max":16384, "center":8192}, // Apple DLS Synth
      ready: function() {
          var self=this;
          var timerId=setInterval(function(){
              var tmp=document.getElementsByTagName("x-webmidirequestaccess");
              self.midiAccess=tmp[0];
              if(self.midiAccess.ready.output==true) {
                  clearInterval(timerId);
                  var elem=self.$["midiout"];
                  self.midiAccess.addOptions("output", elem);
                  self.$["midiout"].addEventListener("change", function(event){
                      self.outputIdx=event.target.value;
                      if(event.target.value=="") self.outputIdx=false;
                      self.fire("midioutput-updated:"+self.id);
                  });
              }
          }, 100);
      },
      initializePerformanceNow: function() {
          this.pfmNow=window.performance.now();
      },
      checkOutputIdx: function() {
          if(this.outputIdx==="false") {
              console.log("output port is NOT selected.");
              return "false";
          }
          return "true";
      },
      getOutputDevice: function() {
          if(this.checkOutputIdx()=="false") {
              return;
          }
          return this.midiAccess.midi.outputs[this.outputIdx];
      },
      sendRawMessage: function(msg, timestamp) {
          if(this.checkOutputIdx()=="false") {
              return;
          }
          if(typeof timestamp==="undefined") {
              timestamp=0;
          }
          this.initializePerformanceNow();
          this.midiAccess.midi.outputs[this.outputIdx].send(msg, this.pfmNow+timestamp);
      },
      sendHRMessage: function(type, ch, param, timestamp) { //hex format
          if(this.checkOutputIdx()=="false") {
              return;
          }
          var msg=false;
          if(typeof timestamp==="undefined") {
              timestamp=0;
          }
          type=type.toLowerCase();
          switch(type) {
              case "noteon":
                  if(typeof param!="object") {
                      console.log("[noteon: Parameter Error:param must be object] "+param);
                      return;
                  }
                  if(typeof param[0]=="string") {
                      param[0]=this.midiAccess.convertItnl2Key(param[0].toUpperCase());
                  }
                  msg=["0x9"+ch, param[0], param[1]];
                  break;
              case "noteoff":
                  if(typeof param!="object") {
                      console.log("[noteoff: Parameter Error:param must be object] "+param);
                      return;
                  }
                  if(typeof param[0]=="string") {
                      param[0]=this.midiAccess.convertItnl2Key(param[0].toUpperCase());
                  }
                  msg=["0x8"+ch, param[0], param[1]];
                  break;
              case "programchange":
                  msg=["0xc"+ch, param];
                  break;
              case "setpitchbendrange":
                  if(typeof param!="object") {
                      console.log("[setpitchbendvalue: Parameter Error:param must be object] "+param);
                      return;
                  }
                  msg=false;
                  this.pitchBendRange={"min":param[0], "max":param[1], "center":(param[0]+param[1]+1)/2};
                  break;
              case "pitchbend":
                  if(typeof param!="number") {
                      console.log("[pitchbend: Parameter Error:param must be object] "+param);
                      return;
                  }
                  var value = param < this.pitchBendRange.min ? this.pitchBendRange.min : param > this.pitchBendRange.max ? this.pitchBendRange.max : param;
                  var msb=(~~(value/128));
                  var lsb=(value%128);
                  msg=["0xe"+ch, lsb, msb];
                  break;
              case "sustain":
                  var msg=["0xb"+ch, 0x40, 0x00];
                  switch(param) {
                      case "on":
                          msg=["0xb"+ch, 0x40, 0x7f];
                          break;
                  }
                  break;
              case "modulation":
                  if(typeof param!="number") {
                      console.log("[Parameter Error:param must be number] "+param);
                      return;
                  }
                  var value = param < 0 ? 0 : param > 127 ? 127 : param;
                  var msg=["0xb"+ch, 0x01, value];
                  break;
              case "allsoundoff":
                  msg=[ "0xb"+ch, 0x78, 0x00 ];
                  break;
              case "resetallcontroller":
                  msg=[ "0xb"+ch, 0x79, 0x00 ];
                  break;
              case "allnoteoff":
                  msg=[ "0xb"+ch, 0x7b, 0x00 ];
                  break;
          }
          // send message
          if(msg!=false) {
              this.initializePerformanceNow();
              this.midiAccess.midi.outputs[this.outputIdx].send(msg, this.pfmNow+timestamp);
          }
      }

  });
