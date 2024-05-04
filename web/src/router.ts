import { createWebHistory, createRouter } from "vue-router";
import { RouteRecordRaw } from "vue-router";

const routes: Array<RouteRecordRaw> = [
  {
    path: "/",
    name: "controlPage",
    component: () => import("./components/ControlPage.vue"),
  }
];

const router = createRouter({
  history: createWebHistory(),
  routes,
});

export default router;
