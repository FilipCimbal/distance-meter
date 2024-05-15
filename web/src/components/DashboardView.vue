<template>
  <div>
    <div class="hello">
      {{ Title }}
    </div>
    <div class="slidecontainer">
      <input type="range" min="0" max="10" step="0.1" v-model="flowRequest" class="slider">      
    </div>
    <br><br>
    <br><br>
    <div class="leftDiv">
      <table>
      <tr>
      <td class="tabCell">Nastavená rychlost proudění:</td>
      <td class="tabValueCell"><input class="input_1" v-model="flowRequest"> m/s</td>
      </tr>        
      <tr>
      <td class="tabCell">Naměřená rychlost proudění:</td>
      <td class="tabValueCell"> {{ speedCurrent.toFixed(2) }} m/s</td>
      </tr>
      <tr>
      <td class="tabCell">Naměřený průtok: </td>        
      <td class="tabValueCell">{{ flowCurrent }} m<sup>3</sup>/h</td>
      </tr>
      </table>
    </div>
  </div>
</template>

<script lang="ts">

import { defineComponent, ref, warn } from 'vue'
import ApiConnector  from '../services/ApiConnector'


export default defineComponent({
  props:
  {
    msg: String
  },
  setup(props) {

    const Title = ref(props.msg)
    const flowRequest = ref(2)
    const flowCurrent = ref(0)
    const speedCurrent = ref(0)

    const connector = new ApiConnector(null)

    const refreshTap = () => {

      setTimeout(async () => {

        try {
          let response = await connector.setSpeed(flowRequest.value)
          console.warn(response)
          response = await connector.getFlow()
          if (response.args.speed)
          {
            flowCurrent.value = response.args.flow
            speedCurrent.value = response.args.speed
          }
          console.warn(response)
        } catch (error) {
          console.error(error)
        }

        refreshTap()


      }, 1000)

    }    

    refreshTap()

    return {
      Title,
      flowRequest,
      flowCurrent,
      speedCurrent
    }
  }
})
</script>

<style>
.input_1 {
  width: 20px
}

h3 {
  margin: 40px 0 0;
}

ul {
  list-style-type: none;
  padding: 0;
}

li {
  display: inline-block;
  margin: 0 10px;
}

a {
  color: #42b983;
}

.slidecontainer {
  width: 100%;
  /* Width of the outside container */
}

/* The slider itself */
.slider {
  -webkit-appearance: none;
  /* Override default CSS styles */
  appearance: none;
  width: 100%;
  /* Full-width */
  height: 15px;
  /* Specified height */
  background: #d3d3d3;
  /* Grey background */
  outline: none;
  /* Remove outline */
  opacity: 0.7;
  /* Set transparency (for mouse-over effects on hover) */
  -webkit-transition: .2s;
  /* 0.2 seconds transition on hover */
  transition: opacity .2s;
}

/* Mouse-over effects */
.slider:hover {
  opacity: 1;
  /* Fully shown on mouse-over */
}

/* The slider handle (use -webkit- (Chrome, Opera, Safari, Edge) and -moz- (Firefox) to override default look) */
.slider::-webkit-slider-thumb {
  -webkit-appearance: none;
  /* Override default look */
  appearance: none;
  width: 10px;
  /* Set a specific slider handle width */
  height: 55px;
  /* Slider handle height */
  background: #021d13;
  /* Green background */
  cursor: pointer;
  /* Cursor on hover */
}

.slider::-moz-range-thumb {
  width: 10px;
  /* Set a specific slider handle width */
  height: 55px;
  /* Slider handle height */
  background: #0d4f7a;
  /* Green background */
  cursor: pointer;
  /* Cursor on hover */
}

.leftDiv {
  text-align: left;

}

.tabCell
{
  width: 300px;
  font-weight: bold;
  font-size: large;
}

.tabValueCell
{
  font-weight: normal;
  font-size: large;
}
</style>
