<template>
  <!-- <img alt="Vue logo" src="./assets/logo.png" /> -->
  <DashboardView msg="Flow meter" />
</template>

<script lang="ts">
import { defineComponent } from "vue";
import DashboardView from "./components/DashboardView.vue";

export default defineComponent({
  name: "App",
  components: {
    DashboardView,
  },
});
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
