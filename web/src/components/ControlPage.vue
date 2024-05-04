<template>
  <div class="list row">
    <div class="col-md-8">
      {{ pageName }}
    </div>
  </div>
</template>

<script lang="ts">
import {defineComponent, onUnmounted, ref, watch} from 'vue'
import TutorialDataService from "@/services/TutorialDataService";
import Tutorial from "@/types/Tutorial";
import ResponseData from "@/types/ResponseData";

export default defineComponent({
  props: {
    modelValue: {
      type: Boolean,
    }
  },
  setup () {

    const pageName = ref<string>('Controller')


    // watch(() => code.value, () => {

    // })

    return {
      pageName
    }
  }
})
</script>

<style>
.list {
  text-align: left;
  max-width: 750px;
  margin: auto;
}
</style>
