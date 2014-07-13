<?hh

/*
 * This file is part of elliotwright.co
 *
 * (c) Elliot Wright <elliot@elliotwright.co>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace SeerUK\Module\BlogModule\Controller;

use SeerUK\Module\BlogModule\Data\Entity\Article;
use SeerUK\Module\BlogModule\Form\Type\ArticleType;
use Symfony\Component\HttpFoundation\Response;
use Trident\Component\HttpKernel\Exception\NotFoundHttpException;
use Trident\Module\FrameworkModule\Controller\Controller;

/**
 * Blog Controller
 *
 * Controls the homepage of the portfolio/blog
 *
 * @author Elliot Wright <elliot@elliotwright.co>
 */
class BlogController extends Controller
{
    /**
     * View a blog article
     *
     * @param integer $id
     *
     * @return Response
     */
    public function viewAction($id): Response
    {
        return $this->get('caching.proxy')->proxy("rendered.blog.article.$id",
            function() use ($id) {
                $twig = $this->get('bm.templating.engine.twig.string');
                $ar   = $this->get('bm.repository.article');

                $article = $ar->findOneById($id);

                if ( ! $article) {
                    throw new NotFoundHttpException(sprintf(Article::ERR_NO_ARTICLE, $id));
                }

                // Render the content and set it inside the article for view
                $rendered = $twig->render('{% markdown %}'.$article->getContent().'{% endmarkdown %}');
                $article->setContent($rendered);

                return $this->render('SeerUKBlogModule:Blog:view.html.twig', [
                    'article' => $article
                ]);
            },
        3600);
    }

    /**
     * Edit a blog article
     *
     * @param integer $id
     *
     * @return Response
     */
    public function editAction($id): Response
    {
        $ar = $this->get('bm.repository.article');
        $em = $this->get('doctrine.orm.entity_manager');
        $ff = $this->get('form.factory');

        $article = $ar->findOneById($id);

        if ( ! $article) {
            throw new NotFoundHttpException(sprintf(Article::ERR_NO_ARTICLE, $id));
        }

        $form = $ff->create(new ArticleType, $article);
        $form->handleRequest($this->get('request'));

        if ($form->isValid()) {
            $caching = $this->get('caching');
            $article = $form->getData();

            $em->persist($article);
            $em->flush();

            $caching->remove("rendered.blog.article.$id");

            return $this->redirect($this->generateUrl('bm_blog_view', [
                'id' => $article->getId()
            ]));
        }

        return $this->render('SeerUKBlogModule:Blog:edit.html.twig', [
            'form' => $form->createView(),
        ]);
    }
}
